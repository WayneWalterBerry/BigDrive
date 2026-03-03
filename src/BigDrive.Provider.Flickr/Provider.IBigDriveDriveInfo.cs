// <copyright file="Provider.IBigDriveDriveInfo.cs" company="Wayne Walter Berry">
// Copyright (c) Wayne Walter Berry. All rights reserved.
// </copyright>

namespace BigDrive.Provider.Flickr
{
    using BigDrive.Interfaces;
    using BigDrive.Interfaces.Model;
    using BigDrive.Interfaces.Serialization;

    /// <summary>
    /// Implementation of <see cref="IBigDriveDriveInfo"/> for the Flickr provider.
    /// Declares the custom parameters required when mounting a Flickr drive.
    /// </summary>
    public partial class Provider
    {
        /// <summary>
        /// Gets the parameter definitions required by the Flickr provider when mounting a new drive.
        /// </summary>
        /// <returns>
        /// A JSON string containing an array of parameter definitions
        /// for the FlickrApiKey and FlickrApiSecret properties.
        /// </returns>
        public string GetDriveParameters()
        {
            DriveParameterDefinition[] parameters = new DriveParameterDefinition[]
            {
                new DriveParameterDefinition
                {
                    Name = "FlickrApiKey",
                    Description = "Your Flickr API key (from https://www.flickr.com/services/apps/create/).",
                    Type = DriveParameterType.String
                },
                new DriveParameterDefinition
                {
                    Name = "FlickrApiSecret",
                    Description = "Your Flickr API secret (from https://www.flickr.com/services/apps/create/).",
                    Type = DriveParameterType.Secret
                }
            };

            return DriveParameterSerializer.Serialize(parameters);
        }
    }
}
