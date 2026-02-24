// <copyright file="OAuthHelper.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Shell
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Sockets;
    using System.Security.Cryptography;
    using System.Text;
    using System.Threading;
    using System.Web;

    /// <summary>
    /// Helper class for performing OAuth authentication flows in CLI applications.
    /// Implements patterns from OAuth Authentication Patterns research document.
    /// </summary>
    /// <remarks>
    /// Supports two primary flows:
    /// 1. Authorization Code with Loopback - Opens browser, listens on localhost for redirect
    /// 2. Device Code - Prints URL and code for user to enter on any device
    /// 
    /// Based on patterns used by Azure CLI, GitHub CLI, Google Cloud SDK, AWS CLI, and Rclone.
    /// </remarks>
    public static class OAuthHelper
    {
        /// <summary>
        /// The default timeout for OAuth flows in seconds.
        /// </summary>
        private const int DefaultTimeoutSeconds = 300;

        /// <summary>
        /// Performs OAuth 2.0 Authorization Code flow with loopback redirect.
        /// </summary>
        /// <param name="authorizationUrl">The authorization endpoint URL.</param>
        /// <param name="tokenUrl">The token endpoint URL.</param>
        /// <param name="clientId">The OAuth client ID.</param>
        /// <param name="clientSecret">The OAuth client secret (may be empty).</param>
        /// <param name="scopes">The scopes to request (space-separated).</param>
        /// <param name="providerName">The friendly name for display.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>OAuth result with tokens, or null if failed.</returns>
        public static OAuthResult PerformAuthorizationCodeFlow(
            string authorizationUrl,
            string tokenUrl,
            string clientId,
            string clientSecret,
            string scopes,
            string providerName,
            CancellationToken cancellationToken)
        {
            int port = GetAvailablePort();
            string redirectUri = string.Format("http://127.0.0.1:{0}/", port);
            string state = GenerateState();

            // Build authorization URL
            string fullAuthUrl = BuildAuthorizationUrl(authorizationUrl, clientId, redirectUri, scopes, state);

            Console.WriteLine();
            Console.WriteLine("Opening your web browser for {0} login...", providerName);
            Console.WriteLine();
            Console.WriteLine("If the browser does not open automatically, navigate to:");
            Console.WriteLine("  {0}", fullAuthUrl);
            Console.WriteLine();
            Console.WriteLine("Waiting for authentication... Press Ctrl+C to cancel.");
            Console.WriteLine();

            // Start local HTTP listener
            HttpListener listener = new HttpListener();
            listener.Prefixes.Add(redirectUri);

            try
            {
                listener.Start();

                // Open browser
                OpenBrowser(fullAuthUrl);

                // Wait for callback with timeout
                IAsyncResult result = listener.BeginGetContext(null, null);

                int timeoutMs = DefaultTimeoutSeconds * 1000;
                int waitResult = WaitHandle.WaitAny(
                    new WaitHandle[] { result.AsyncWaitHandle, cancellationToken.WaitHandle },
                    timeoutMs);

                if (waitResult == WaitHandle.WaitTimeout)
                {
                    Console.WriteLine("Authentication timed out.");
                    Console.WriteLine("You can try again with 'login' or use 'login --device-code' for headless environments.");
                    return null;
                }

                if (cancellationToken.IsCancellationRequested)
                {
                    Console.WriteLine("Authentication cancelled.");
                    return null;
                }

                HttpListenerContext context = listener.EndGetContext(result);
                string authCode = ExtractAuthorizationCode(context, state);

                // Send success response to browser
                SendBrowserResponse(context, providerName, true);

                if (string.IsNullOrEmpty(authCode))
                {
                    Console.WriteLine("Authentication failed: No authorization code received.");
                    return null;
                }

                Console.WriteLine("Got authorization code. Exchanging for tokens...");

                // Exchange code for tokens
                OAuthResult oauthResult = ExchangeCodeForTokens(tokenUrl, clientId, clientSecret, authCode, redirectUri);

                if (oauthResult != null)
                {
                    Console.WriteLine("Authentication successful!");
                }
                else
                {
                    Console.WriteLine("Failed to exchange authorization code for tokens.");
                }

                return oauthResult;
            }
            catch (HttpListenerException ex)
            {
                Console.WriteLine("Error starting local HTTP listener: {0}", ex.Message);
                Console.WriteLine("The loopback flow requires ability to listen on localhost.");
                Console.WriteLine("Try 'login --device-code' as an alternative.");
                return null;
            }
            finally
            {
                if (listener.IsListening)
                {
                    listener.Stop();
                }

                listener.Close();
            }
        }

        /// <summary>
        /// Performs OAuth 2.0 Device Authorization Grant (RFC 8628).
        /// </summary>
        /// <param name="deviceAuthorizationUrl">The device authorization endpoint URL.</param>
        /// <param name="tokenUrl">The token endpoint URL.</param>
        /// <param name="clientId">The OAuth client ID.</param>
        /// <param name="scopes">The scopes to request.</param>
        /// <param name="providerName">The friendly name for display.</param>
        /// <param name="cancellationToken">Cancellation token.</param>
        /// <returns>OAuth result with tokens, or null if failed.</returns>
        public static OAuthResult PerformDeviceCodeFlow(
            string deviceAuthorizationUrl,
            string tokenUrl,
            string clientId,
            string scopes,
            string providerName,
            CancellationToken cancellationToken)
        {
            // Request device code
            DeviceCodeResponse deviceCode = RequestDeviceCode(deviceAuthorizationUrl, clientId, scopes);

            if (deviceCode == null)
            {
                Console.WriteLine("Failed to request device code from {0}.", providerName);
                return null;
            }

            Console.WriteLine();
            Console.WriteLine("To sign in to {0}:", providerName);
            Console.WriteLine();
            Console.WriteLine("  1. Open a web browser and go to: {0}", deviceCode.VerificationUri);
            Console.WriteLine("  2. Enter the code: {0}", deviceCode.UserCode);
            Console.WriteLine();

            // Try to copy code to clipboard
            if (TryCopyToClipboard(deviceCode.UserCode))
            {
                Console.WriteLine("  (Code has been copied to your clipboard)");
                Console.WriteLine();
            }

            Console.WriteLine("Waiting for you to authorize... Press Ctrl+C to cancel.");
            Console.WriteLine();

            // Poll for completion
            int interval = deviceCode.Interval > 0 ? deviceCode.Interval : 5;
            int expiresIn = deviceCode.ExpiresIn > 0 ? deviceCode.ExpiresIn : 600;
            DateTime expiration = DateTime.Now.AddSeconds(expiresIn);

            while (DateTime.Now < expiration && !cancellationToken.IsCancellationRequested)
            {
                Thread.Sleep(interval * 1000);

                OAuthResult result = PollDeviceCodeToken(tokenUrl, clientId, deviceCode.DeviceCode);

                if (result != null)
                {
                    Console.WriteLine("Authentication successful!");
                    return result;
                }

                // Still pending, continue polling
                Console.Write(".");
            }

            Console.WriteLine();

            if (cancellationToken.IsCancellationRequested)
            {
                Console.WriteLine("Authentication cancelled.");
            }
            else
            {
                Console.WriteLine("Device code expired. Please try again.");
            }

            return null;
        }

        /// <summary>
        /// Gets an available local port for the loopback listener.
        /// </summary>
        /// <returns>An available port number.</returns>
        private static int GetAvailablePort()
        {
            TcpListener listener = new TcpListener(IPAddress.Loopback, 0);
            listener.Start();
            int port = ((IPEndPoint)listener.LocalEndpoint).Port;
            listener.Stop();
            return port;
        }

        /// <summary>
        /// Generates a random state parameter for CSRF protection.
        /// </summary>
        /// <returns>A random state string.</returns>
        private static string GenerateState()
        {
            byte[] bytes = new byte[32];
            using (RNGCryptoServiceProvider rng = new RNGCryptoServiceProvider())
            {
                rng.GetBytes(bytes);
            }

            return Convert.ToBase64String(bytes)
                .Replace("+", "-")
                .Replace("/", "_")
                .Replace("=", "");
        }

        /// <summary>
        /// Builds the full authorization URL with query parameters.
        /// </summary>
        private static string BuildAuthorizationUrl(
            string baseUrl,
            string clientId,
            string redirectUri,
            string scopes,
            string state)
        {
            StringBuilder sb = new StringBuilder(baseUrl);

            if (baseUrl.Contains("?"))
            {
                sb.Append("&");
            }
            else
            {
                sb.Append("?");
            }

            sb.AppendFormat("response_type=code");
            sb.AppendFormat("&client_id={0}", Uri.EscapeDataString(clientId));
            sb.AppendFormat("&redirect_uri={0}", Uri.EscapeDataString(redirectUri));

            if (!string.IsNullOrEmpty(scopes))
            {
                sb.AppendFormat("&scope={0}", Uri.EscapeDataString(scopes));
            }

            sb.AppendFormat("&state={0}", Uri.EscapeDataString(state));

            return sb.ToString();
        }

        /// <summary>
        /// Opens the system default browser to the specified URL.
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
                // Browser may not be available
                Console.WriteLine("Could not open browser automatically.");
            }
        }

        /// <summary>
        /// Extracts the authorization code from the callback request.
        /// </summary>
        private static string ExtractAuthorizationCode(HttpListenerContext context, string expectedState)
        {
            string query = context.Request.Url.Query;

            if (string.IsNullOrEmpty(query))
            {
                return null;
            }

            System.Collections.Specialized.NameValueCollection queryParams =
                HttpUtility.ParseQueryString(query);

            // Check state parameter for CSRF protection
            string returnedState = queryParams["state"];
            if (!string.Equals(returnedState, expectedState, StringComparison.Ordinal))
            {
                Console.WriteLine("Warning: State parameter mismatch. Possible CSRF attack.");
                return null;
            }

            // Check for error
            string error = queryParams["error"];
            if (!string.IsNullOrEmpty(error))
            {
                string errorDescription = queryParams["error_description"] ?? error;
                Console.WriteLine("Authorization error: {0}", errorDescription);
                return null;
            }

            return queryParams["code"];
        }

        /// <summary>
        /// Sends a response to the browser after OAuth callback.
        /// </summary>
        private static void SendBrowserResponse(HttpListenerContext context, string providerName, bool success)
        {
            string html;

            if (success)
            {
                html = string.Format(@"<!DOCTYPE html>
<html>
<head><title>BigDrive - Authentication Successful</title></head>
<body style='font-family: Arial, sans-serif; text-align: center; padding: 50px;'>
    <h1 style='color: #28a745;'>&#10003; Authentication Successful</h1>
    <p>You have successfully authenticated with {0}.</p>
    <p>You can close this window and return to BigDrive Shell.</p>
</body>
</html>", providerName);
            }
            else
            {
                html = string.Format(@"<!DOCTYPE html>
<html>
<head><title>BigDrive - Authentication Failed</title></head>
<body style='font-family: Arial, sans-serif; text-align: center; padding: 50px;'>
    <h1 style='color: #dc3545;'>&#10007; Authentication Failed</h1>
    <p>There was a problem authenticating with {0}.</p>
    <p>Please return to BigDrive Shell and try again.</p>
</body>
</html>", providerName);
            }

            byte[] buffer = Encoding.UTF8.GetBytes(html);
            context.Response.ContentType = "text/html";
            context.Response.ContentLength64 = buffer.Length;
            context.Response.OutputStream.Write(buffer, 0, buffer.Length);
            context.Response.OutputStream.Close();
        }

        /// <summary>
        /// Exchanges an authorization code for access and refresh tokens.
        /// </summary>
        private static OAuthResult ExchangeCodeForTokens(
            string tokenUrl,
            string clientId,
            string clientSecret,
            string authCode,
            string redirectUri)
        {
            try
            {
                using (WebClient client = new WebClient())
                {
                    client.Headers[HttpRequestHeader.ContentType] = "application/x-www-form-urlencoded";

                    StringBuilder postData = new StringBuilder();
                    postData.AppendFormat("grant_type=authorization_code");
                    postData.AppendFormat("&code={0}", Uri.EscapeDataString(authCode));
                    postData.AppendFormat("&client_id={0}", Uri.EscapeDataString(clientId));
                    postData.AppendFormat("&redirect_uri={0}", Uri.EscapeDataString(redirectUri));

                    if (!string.IsNullOrEmpty(clientSecret))
                    {
                        postData.AppendFormat("&client_secret={0}", Uri.EscapeDataString(clientSecret));
                    }

                    string response = client.UploadString(tokenUrl, postData.ToString());
                    return ParseTokenResponse(response);
                }
            }
            catch (WebException ex)
            {
                Console.WriteLine("Error exchanging code for tokens: {0}", ex.Message);
                return null;
            }
        }

        /// <summary>
        /// Requests a device code from the authorization server.
        /// </summary>
        private static DeviceCodeResponse RequestDeviceCode(string deviceAuthUrl, string clientId, string scopes)
        {
            try
            {
                using (WebClient client = new WebClient())
                {
                    client.Headers[HttpRequestHeader.ContentType] = "application/x-www-form-urlencoded";

                    StringBuilder postData = new StringBuilder();
                    postData.AppendFormat("client_id={0}", Uri.EscapeDataString(clientId));

                    if (!string.IsNullOrEmpty(scopes))
                    {
                        postData.AppendFormat("&scope={0}", Uri.EscapeDataString(scopes));
                    }

                    string response = client.UploadString(deviceAuthUrl, postData.ToString());
                    return ParseDeviceCodeResponse(response);
                }
            }
            catch (WebException ex)
            {
                Console.WriteLine("Error requesting device code: {0}", ex.Message);
                return null;
            }
        }

        /// <summary>
        /// Polls the token endpoint for device code completion.
        /// </summary>
        private static OAuthResult PollDeviceCodeToken(string tokenUrl, string clientId, string deviceCode)
        {
            try
            {
                using (WebClient client = new WebClient())
                {
                    client.Headers[HttpRequestHeader.ContentType] = "application/x-www-form-urlencoded";

                    StringBuilder postData = new StringBuilder();
                    postData.AppendFormat("grant_type=urn:ietf:params:oauth:grant-type:device_code");
                    postData.AppendFormat("&client_id={0}", Uri.EscapeDataString(clientId));
                    postData.AppendFormat("&device_code={0}", Uri.EscapeDataString(deviceCode));

                    string response = client.UploadString(tokenUrl, postData.ToString());

                    // Check for pending/slow_down errors
                    if (response.Contains("authorization_pending") || response.Contains("slow_down"))
                    {
                        return null; // Still pending
                    }

                    return ParseTokenResponse(response);
                }
            }
            catch (WebException)
            {
                // Polling error, continue
                return null;
            }
        }

        /// <summary>
        /// Parses a JSON token response.
        /// </summary>
        private static OAuthResult ParseTokenResponse(string json)
        {
            // Simple JSON parsing without external dependencies
            OAuthResult result = new OAuthResult();

            result.AccessToken = ExtractJsonValue(json, "access_token");
            result.RefreshToken = ExtractJsonValue(json, "refresh_token");
            result.TokenType = ExtractJsonValue(json, "token_type");

            string expiresInStr = ExtractJsonValue(json, "expires_in");
            if (int.TryParse(expiresInStr, out int expiresIn))
            {
                result.ExpiresIn = expiresIn;
            }

            // For OAuth 1.0a (Flickr), also check for oauth_token/oauth_token_secret
            if (string.IsNullOrEmpty(result.AccessToken))
            {
                result.AccessToken = ExtractJsonValue(json, "oauth_token");
                result.AccessTokenSecret = ExtractJsonValue(json, "oauth_token_secret");
            }

            if (string.IsNullOrEmpty(result.AccessToken))
            {
                return null;
            }

            return result;
        }

        /// <summary>
        /// Parses a device code response.
        /// </summary>
        private static DeviceCodeResponse ParseDeviceCodeResponse(string json)
        {
            DeviceCodeResponse response = new DeviceCodeResponse();

            response.DeviceCode = ExtractJsonValue(json, "device_code");
            response.UserCode = ExtractJsonValue(json, "user_code");
            response.VerificationUri = ExtractJsonValue(json, "verification_uri");

            if (string.IsNullOrEmpty(response.VerificationUri))
            {
                response.VerificationUri = ExtractJsonValue(json, "verification_url");
            }

            string intervalStr = ExtractJsonValue(json, "interval");
            if (int.TryParse(intervalStr, out int interval))
            {
                response.Interval = interval;
            }

            string expiresInStr = ExtractJsonValue(json, "expires_in");
            if (int.TryParse(expiresInStr, out int expiresIn))
            {
                response.ExpiresIn = expiresIn;
            }

            if (string.IsNullOrEmpty(response.DeviceCode) || string.IsNullOrEmpty(response.UserCode))
            {
                return null;
            }

            return response;
        }

        /// <summary>
        /// Extracts a value from a JSON string (simple parser).
        /// </summary>
        private static string ExtractJsonValue(string json, string key)
        {
            string searchPattern = string.Format("\"{0}\"", key);
            int keyIndex = json.IndexOf(searchPattern, StringComparison.OrdinalIgnoreCase);

            if (keyIndex < 0)
            {
                return null;
            }

            int colonIndex = json.IndexOf(':', keyIndex + searchPattern.Length);
            if (colonIndex < 0)
            {
                return null;
            }

            int valueStart = colonIndex + 1;
            while (valueStart < json.Length && char.IsWhiteSpace(json[valueStart]))
            {
                valueStart++;
            }

            if (valueStart >= json.Length)
            {
                return null;
            }

            // String value
            if (json[valueStart] == '"')
            {
                int valueEnd = json.IndexOf('"', valueStart + 1);
                if (valueEnd < 0)
                {
                    return null;
                }

                return json.Substring(valueStart + 1, valueEnd - valueStart - 1);
            }

            // Numeric or other value
            int endIndex = valueStart;
            while (endIndex < json.Length && json[endIndex] != ',' && json[endIndex] != '}' && json[endIndex] != ']')
            {
                endIndex++;
            }

            return json.Substring(valueStart, endIndex - valueStart).Trim();
        }

        /// <summary>
        /// Tries to copy text to the Windows clipboard.
        /// </summary>
        private static bool TryCopyToClipboard(string text)
        {
            try
            {
                // Use clip.exe for clipboard access (works from console)
                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = "clip.exe",
                    RedirectStandardInput = true,
                    UseShellExecute = false,
                    CreateNoWindow = true
                };

                using (Process process = Process.Start(psi))
                {
                    process.StandardInput.Write(text);
                    process.StandardInput.Close();
                    process.WaitForExit(1000);
                    return process.ExitCode == 0;
                }
            }
            catch
            {
                return false;
            }
        }
    }

    /// <summary>
    /// Result of an OAuth authentication flow.
    /// </summary>
    public class OAuthResult
    {
        /// <summary>
        /// Gets or sets the access token.
        /// </summary>
        public string AccessToken { get; set; }

        /// <summary>
        /// Gets or sets the access token secret (OAuth 1.0a only).
        /// </summary>
        public string AccessTokenSecret { get; set; }

        /// <summary>
        /// Gets or sets the refresh token.
        /// </summary>
        public string RefreshToken { get; set; }

        /// <summary>
        /// Gets or sets the token type (e.g., "Bearer").
        /// </summary>
        public string TokenType { get; set; }

        /// <summary>
        /// Gets or sets the expiration time in seconds.
        /// </summary>
        public int ExpiresIn { get; set; }
    }

    /// <summary>
    /// Response from a device code request.
    /// </summary>
    internal class DeviceCodeResponse
    {
        /// <summary>
        /// Gets or sets the device code.
        /// </summary>
        public string DeviceCode { get; set; }

        /// <summary>
        /// Gets or sets the user code to display.
        /// </summary>
        public string UserCode { get; set; }

        /// <summary>
        /// Gets or sets the verification URI.
        /// </summary>
        public string VerificationUri { get; set; }

        /// <summary>
        /// Gets or sets the polling interval in seconds.
        /// </summary>
        public int Interval { get; set; }

        /// <summary>
        /// Gets or sets the expiration time in seconds.
        /// </summary>
        public int ExpiresIn { get; set; }
    }
}
