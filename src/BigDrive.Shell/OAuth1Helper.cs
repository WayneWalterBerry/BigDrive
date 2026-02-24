// <copyright file="OAuth1Helper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Net;
    using System.Security.Cryptography;
    using System.Text;
    using System.Threading;

    /// <summary>
    /// Helper class for performing OAuth 1.0a authentication flows.
    /// Used by providers like Flickr that still use OAuth 1.0a.
    /// </summary>
    /// <remarks>
    /// OAuth 1.0a is an older protocol that requires request token acquisition,
    /// user authorization, and then token exchange. This flow uses the manual
    /// verifier code approach as recommended in the research document.
    /// </remarks>
    public static class OAuth1Helper
    {
        /// <summary>
        /// Performs OAuth 1.0a authentication flow.
        /// </summary>
        /// <param name="requestTokenUrl">The request token endpoint URL.</param>
        /// <param name="authorizeUrl">The user authorization URL.</param>
        /// <param name="accessTokenUrl">The access token endpoint URL.</param>
        /// <param name="consumerKey">The OAuth consumer key (API key).</param>
        /// <param name="consumerSecret">The OAuth consumer secret.</param>
        /// <param name="providerName">The friendly name for display.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>OAuth result with tokens, or null if failed.</returns>
        public static OAuthResult PerformOAuth1Flow(
            string requestTokenUrl,
            string authorizeUrl,
            string accessTokenUrl,
            string consumerKey,
            string consumerSecret,
            string providerName,
            CancellationToken cancellationToken)
        {
            // Step 1: Get request token
            Console.WriteLine("Requesting temporary token from {0}...", providerName);

            OAuth1Token requestToken = GetRequestToken(
                requestTokenUrl, consumerKey, consumerSecret);

            if (requestToken == null)
            {
                Console.WriteLine("Failed to get request token from {0}.", providerName);
                return null;
            }

            // Step 2: Direct user to authorize
            string authUrl = string.Format("{0}?oauth_token={1}",
                authorizeUrl, Uri.EscapeDataString(requestToken.Token));

            Console.WriteLine();
            Console.WriteLine("Opening your web browser for {0} authorization...", providerName);
            Console.WriteLine();
            Console.WriteLine("If the browser does not open automatically, navigate to:");
            Console.WriteLine("  {0}", authUrl);
            Console.WriteLine();

            OpenBrowser(authUrl);

            // Step 3: Get verifier from user
            Console.WriteLine("After authorizing, {0} will display a verification code.", providerName);
            Console.Write("Enter the verification code: ");

            string verifier = ReadVerifierCode(cancellationToken);

            if (string.IsNullOrEmpty(verifier))
            {
                Console.WriteLine();
                Console.WriteLine("Authentication cancelled or no verifier entered.");
                return null;
            }

            Console.WriteLine();
            Console.WriteLine("Exchanging for access token...");

            // Step 4: Exchange for access token
            OAuthResult result = GetAccessToken(
                accessTokenUrl,
                consumerKey,
                consumerSecret,
                requestToken.Token,
                requestToken.TokenSecret,
                verifier);

            if (result != null)
            {
                Console.WriteLine("Authentication successful!");
            }
            else
            {
                Console.WriteLine("Failed to get access token.");
            }

            return result;
        }

        /// <summary>
        /// Gets a request token from the OAuth 1.0a provider.
        /// </summary>
        private static OAuth1Token GetRequestToken(
            string requestTokenUrl,
            string consumerKey,
            string consumerSecret)
        {
            try
            {
                string timestamp = GetTimestamp();
                string nonce = GetNonce();

                SortedDictionary<string, string> parameters = new SortedDictionary<string, string>
                {
                    { "oauth_consumer_key", consumerKey },
                    { "oauth_nonce", nonce },
                    { "oauth_signature_method", "HMAC-SHA1" },
                    { "oauth_timestamp", timestamp },
                    { "oauth_version", "1.0" },
                    { "oauth_callback", "oob" }
                };

                string signature = GenerateSignature(
                    "POST", requestTokenUrl, parameters, consumerSecret, null);
                parameters["oauth_signature"] = signature;

                string authHeader = BuildAuthorizationHeader(parameters);

                using (WebClient client = new WebClient())
                {
                    client.Headers[HttpRequestHeader.Authorization] = authHeader;
                    string response = client.UploadString(requestTokenUrl, string.Empty);
                    return ParseTokenResponse(response);
                }
            }
            catch (WebException ex)
            {
                Console.WriteLine("Error getting request token: {0}", ex.Message);
                return null;
            }
        }

        /// <summary>
        /// Exchanges request token for access token.
        /// </summary>
        private static OAuthResult GetAccessToken(
            string accessTokenUrl,
            string consumerKey,
            string consumerSecret,
            string requestToken,
            string requestTokenSecret,
            string verifier)
        {
            try
            {
                string timestamp = GetTimestamp();
                string nonce = GetNonce();

                SortedDictionary<string, string> parameters = new SortedDictionary<string, string>
                {
                    { "oauth_consumer_key", consumerKey },
                    { "oauth_nonce", nonce },
                    { "oauth_signature_method", "HMAC-SHA1" },
                    { "oauth_timestamp", timestamp },
                    { "oauth_version", "1.0" },
                    { "oauth_token", requestToken },
                    { "oauth_verifier", verifier }
                };

                string signature = GenerateSignature(
                    "POST", accessTokenUrl, parameters, consumerSecret, requestTokenSecret);
                parameters["oauth_signature"] = signature;

                string authHeader = BuildAuthorizationHeader(parameters);

                using (WebClient client = new WebClient())
                {
                    client.Headers[HttpRequestHeader.Authorization] = authHeader;
                    string response = client.UploadString(accessTokenUrl, string.Empty);

                    OAuth1Token token = ParseTokenResponse(response);
                    if (token != null)
                    {
                        return new OAuthResult
                        {
                            AccessToken = token.Token,
                            AccessTokenSecret = token.TokenSecret
                        };
                    }

                    return null;
                }
            }
            catch (WebException ex)
            {
                Console.WriteLine("Error getting access token: {0}", ex.Message);
                return null;
            }
        }

        /// <summary>
        /// Generates an OAuth 1.0a signature.
        /// </summary>
        private static string GenerateSignature(
            string httpMethod,
            string url,
            SortedDictionary<string, string> parameters,
            string consumerSecret,
            string tokenSecret)
        {
            // Build parameter string
            StringBuilder paramString = new StringBuilder();
            bool first = true;

            foreach (KeyValuePair<string, string> param in parameters)
            {
                if (!first)
                {
                    paramString.Append("&");
                }

                paramString.AppendFormat("{0}={1}",
                    Uri.EscapeDataString(param.Key),
                    Uri.EscapeDataString(param.Value));
                first = false;
            }

            // Build signature base string
            string signatureBase = string.Format("{0}&{1}&{2}",
                httpMethod.ToUpperInvariant(),
                Uri.EscapeDataString(url),
                Uri.EscapeDataString(paramString.ToString()));

            // Build signing key
            string signingKey = string.Format("{0}&{1}",
                Uri.EscapeDataString(consumerSecret),
                string.IsNullOrEmpty(tokenSecret) ? string.Empty : Uri.EscapeDataString(tokenSecret));

            // Generate HMAC-SHA1 signature
            using (HMACSHA1 hmac = new HMACSHA1(Encoding.ASCII.GetBytes(signingKey)))
            {
                byte[] signatureBytes = hmac.ComputeHash(Encoding.ASCII.GetBytes(signatureBase));
                return Convert.ToBase64String(signatureBytes);
            }
        }

        /// <summary>
        /// Builds the OAuth Authorization header.
        /// </summary>
        private static string BuildAuthorizationHeader(SortedDictionary<string, string> parameters)
        {
            StringBuilder header = new StringBuilder("OAuth ");
            bool first = true;

            foreach (KeyValuePair<string, string> param in parameters)
            {
                if (!first)
                {
                    header.Append(", ");
                }

                header.AppendFormat("{0}=\"{1}\"",
                    Uri.EscapeDataString(param.Key),
                    Uri.EscapeDataString(param.Value));
                first = false;
            }

            return header.ToString();
        }

        /// <summary>
        /// Parses OAuth 1.0a token response.
        /// </summary>
        private static OAuth1Token ParseTokenResponse(string response)
        {
            OAuth1Token token = new OAuth1Token();

            string[] parts = response.Split('&');
            foreach (string part in parts)
            {
                string[] keyValue = part.Split('=');
                if (keyValue.Length == 2)
                {
                    string key = Uri.UnescapeDataString(keyValue[0]);
                    string value = Uri.UnescapeDataString(keyValue[1]);

                    if (key == "oauth_token")
                    {
                        token.Token = value;
                    }
                    else if (key == "oauth_token_secret")
                    {
                        token.TokenSecret = value;
                    }
                }
            }

            if (string.IsNullOrEmpty(token.Token))
            {
                return null;
            }

            return token;
        }

        /// <summary>
        /// Gets a Unix timestamp.
        /// </summary>
        private static string GetTimestamp()
        {
            TimeSpan ts = DateTime.UtcNow - new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            return ((long)ts.TotalSeconds).ToString();
        }

        /// <summary>
        /// Gets a random nonce.
        /// </summary>
        private static string GetNonce()
        {
            byte[] bytes = new byte[16];
            using (RNGCryptoServiceProvider rng = new RNGCryptoServiceProvider())
            {
                rng.GetBytes(bytes);
            }

            return Convert.ToBase64String(bytes).Replace("+", "").Replace("/", "").Replace("=", "");
        }

        /// <summary>
        /// Opens the system default browser.
        /// </summary>
        private static void OpenBrowser(string url)
        {
            try
            {
                Process.Start(new ProcessStartInfo
                {
                    FileName = url,
                    UseShellExecute = true
                });
            }
            catch (Exception)
            {
                Console.WriteLine("Could not open browser automatically.");
            }
        }

        /// <summary>
        /// Reads the verifier code from console input.
        /// </summary>
        private static string ReadVerifierCode(CancellationToken cancellationToken)
        {
            StringBuilder input = new StringBuilder();

            while (!cancellationToken.IsCancellationRequested)
            {
                if (Console.KeyAvailable)
                {
                    ConsoleKeyInfo key = Console.ReadKey(true);

                    if (key.Key == ConsoleKey.Enter)
                    {
                        Console.WriteLine();
                        return input.ToString().Trim();
                    }
                    else if (key.Key == ConsoleKey.Backspace && input.Length > 0)
                    {
                        input.Length--;
                        Console.Write("\b \b");
                    }
                    else if (key.Key == ConsoleKey.Escape)
                    {
                        return null;
                    }
                    else if (!char.IsControl(key.KeyChar))
                    {
                        input.Append(key.KeyChar);
                        Console.Write(key.KeyChar);
                    }
                }
                else
                {
                    Thread.Sleep(50);
                }
            }

            return null;
        }

        /// <summary>
        /// OAuth 1.0a token container.
        /// </summary>
        private class OAuth1Token
        {
            public string Token { get; set; }
            public string TokenSecret { get; set; }
        }
    }
}
