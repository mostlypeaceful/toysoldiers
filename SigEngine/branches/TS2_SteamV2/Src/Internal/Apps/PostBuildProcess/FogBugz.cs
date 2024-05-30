using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Text;

namespace PostBuildProcess
{
    class FogBugz
    {
        private string fogbugzURL = null;
        private string fogbugzToken = "";
        private static int apiAccessCount = 0;

        public FogBugz(string fogbugzURL)
        {
            this.fogbugzURL = fogbugzURL; 
        }

        public string CallRESTAPIFiles(ref LogIt logIt, Dictionary<string, string> rgArgs, Dictionary<string, byte[]>[] rgFiles)
        {
            // A total sanity check to prevent us from getting stuck in
            // a crazy loop of updating should that somehow ever happen
            apiAccessCount++;
            if (apiAccessCount > 20)
                throw (new Exception("Way too many FogBugz API calls!"));

            if (fogbugzToken != "")
                rgArgs.Add("token", fogbugzToken);

            string sBoundaryString = GetRandomString(30);
            string sBoundary = "--" + sBoundaryString;
            ASCIIEncoding encoding = new ASCIIEncoding();
            UTF8Encoding utf8encoding = new UTF8Encoding();
            HttpWebRequest http = (HttpWebRequest)HttpWebRequest.Create(fogbugzURL);
            http.Method = "POST";
            http.AllowWriteStreamBuffering = true;
            http.ContentType = "multipart/form-data; boundary=" + sBoundaryString;
            string vbCrLf = "\r\n";

            Queue parts = new Queue();

            // Add all the normal arguments
            foreach (System.Collections.Generic.KeyValuePair<string, string> i in rgArgs)
            {
                parts.Enqueue(encoding.GetBytes(sBoundary + vbCrLf));
                parts.Enqueue(encoding.GetBytes("Content-Type: text/plain; charset=\"utf-8\"" + vbCrLf));
                parts.Enqueue(encoding.GetBytes("Content-Disposition: form-data; name=\"" + i.Key + "\"" + vbCrLf + vbCrLf));
                parts.Enqueue(utf8encoding.GetBytes(i.Value));
                parts.Enqueue(encoding.GetBytes(vbCrLf));
            }

            // Add all the files
            if (rgFiles != null)
            {
                foreach (Dictionary<string, byte[]> j in rgFiles)
                {
                    parts.Enqueue(encoding.GetBytes(sBoundary + vbCrLf));
                    parts.Enqueue(encoding.GetBytes("Content-Disposition: form-data; name=\""));
                    parts.Enqueue(j["name"]);
                    parts.Enqueue(encoding.GetBytes("\"; filename=\""));
                    parts.Enqueue(j["filename"]);
                    parts.Enqueue(encoding.GetBytes("\"" + vbCrLf));
                    parts.Enqueue(encoding.GetBytes("Content-Transfer-Encoding: base64" + vbCrLf));
                    parts.Enqueue(encoding.GetBytes("Content-Type: "));
                    parts.Enqueue(j["contenttype"]);
                    parts.Enqueue(encoding.GetBytes(vbCrLf + vbCrLf));
                    parts.Enqueue(j["data"]);
                    parts.Enqueue(encoding.GetBytes(vbCrLf));
                }
            }

            parts.Enqueue(encoding.GetBytes(sBoundary + "--"));

            // Calculate the content length
            int nContentLength = 0;
            foreach (Byte[] part in parts)
                nContentLength += part.Length;
            http.ContentLength = nContentLength;

            // Write the post
            Stream stream = http.GetRequestStream();
            string sent = "";
            foreach (Byte[] part in parts)
            {
                stream.Write(part, 0, part.Length);
                sent += encoding.GetString(part);
            }
            stream.Close();

            // Read the result
            Stream r = http.GetResponse().GetResponseStream();
            StreamReader reader = new StreamReader(r);
            string retValue = reader.ReadToEnd();
            reader.Close();

            return retValue;
        }

        private string GetRandomString(int nLength)
        {
            string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXTZabcdefghiklmnopqrstuvwxyz";
            string s = "";
            System.Random rand = new System.Random();
            for (int i = 0; i < nLength; i++)
            {
                int rnum = (int)Math.Floor((double)rand.Next(0, chars.Length - 1));
                s += chars.Substring(rnum, 1);
            }
            return s;
        }

        public bool CurrentlyLoggedIn()
        {
            if (fogbugzToken != "")
                return true;

            return false;
        }

        public bool LoginToFogBugz(string username, string password, ref LogIt logIt)
        {
            if (fogbugzToken != "")
                return true;

            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("cmd", "logon");
            args.Add("email", username);
            args.Add("password", password);
            string result = CallRESTAPIFiles(ref logIt, args, null);

            System.Xml.XmlTextReader reader = new System.Xml.XmlTextReader(new StringReader(result));
            System.Xml.XPath.XPathDocument doc = new System.Xml.XPath.XPathDocument(reader);
            System.Xml.XPath.XPathNavigator nav = doc.CreateNavigator();

            fogbugzToken = nav.Evaluate("string(response/token)").ToString();
            if (fogbugzToken == "")
                return false;

            return true;
        }

        public bool LogoutOfFogBugz(ref LogIt logIt)
        {
            if (fogbugzToken == "")
                return true;

            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("cmd", "logoff");
            string result = CallRESTAPIFiles(ref logIt, args, null);

            return true;
        }
    }
}
