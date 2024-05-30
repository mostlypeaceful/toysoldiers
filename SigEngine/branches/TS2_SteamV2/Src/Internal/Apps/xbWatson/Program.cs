#region File Information
//-----------------------------------------------------------------------------
// Program.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Collections.Generic;
using System.IO;
using System.IO.IsolatedStorage;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Windows.Forms;

namespace Atg.Samples.xbWatson
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] arguments)
        {
            List<string> consoles = new List<string>();

            // Check arguments for console names/IP addresses.
            foreach (string argument in arguments)
            {
                if (Forms.ConnectionDialog.IsValidNameOrIP(argument))
                    consoles.Add(argument);
            }
        
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            
            // NOTE: Accessing settings before this point will not work.
            LoadAllSettings();
            Application.Run(new Forms.MainForm(consoles));
            SaveAllSettings();
        }
        
        #region Isolated Storage
        
        private const string IsolatedStorageFileName = @"xbWatson.settings";

        private static Dictionary<object, object> settings = new Dictionary<object, object>();

        /// <summary>
        /// Retrieves a particular setting by name from per-user isolated storage.
        /// </summary>
        public static T GetSetting<T>(object key, T defaultValue)
        {
            if (settings.ContainsKey(key))
            {
                object o = settings[key];
                if ((o != null) && (o is T))
                    return (T)o;
            }

            return defaultValue;
        }

        /// <summary>
        /// Retrieves a particular setting by name from per-user isolated storage.
        /// </summary>
        public static void SetSetting<T>(object key, T value)
        {
            settings[key] = value;
        }
        
        /// <summary>
        /// Loads the application's settings from per-user isolated storage.
        /// </summary>
        private static void LoadAllSettings()
        {
            IsolatedStorageFile file = null;
            IsolatedStorageFileStream stream = null;
            
            try
            {
                // Open isolated storage.
                file = IsolatedStorageFile.GetUserStoreForAssembly();
                stream = new IsolatedStorageFileStream(IsolatedStorageFileName,
                    FileMode.Open, FileAccess.Read, file);
                
                // Read settings from storage.
                BinaryFormatter formatter = new BinaryFormatter();
                object o = formatter.Deserialize(stream);
                if (o is Dictionary<object, object>)
                {
                    settings = (Dictionary<object, object>)o;
                }
            }
            catch (FileNotFoundException) { }
            catch (SerializationException) { }
            finally
            {
                if (stream != null)
                    stream.Close();
                if (file != null)
                    file.Close();
            }
        }

        /// <summary>
        /// Saves the application's settings to per-user isolated storage.
        /// </summary>
        private static void SaveAllSettings()
        {
            IsolatedStorageFile file = null;
            IsolatedStorageFileStream stream = null;

            try
            {
                // Open isolated storage.
                file = IsolatedStorageFile.GetUserStoreForAssembly();
                stream = new IsolatedStorageFileStream(IsolatedStorageFileName,
                    FileMode.Create, FileAccess.Write, file);

                // Read settings from storage.
                BinaryFormatter formatter = new BinaryFormatter();
                formatter.Serialize(stream, settings);
            }
            finally
            {
                if (stream != null)
                    stream.Close();
                if (file != null)
                    file.Close();
            }
        }
        
        #endregion
    }
}