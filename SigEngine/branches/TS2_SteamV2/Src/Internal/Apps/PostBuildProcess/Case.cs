using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.XPath;
using System.Xml;

namespace PostBuildProcess
{
    class Case
    {
        public bool active = false;
        public string project = "";
        public string lastUpdate = "";

        public Case(string caseID, ref FogBugz fogBugz, ref LogIt logIt)
        {
            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("q", caseID);
            args.Add("cols", "ixStatus,sProject,sLatestTextSummary");
            args.Add("cmd", "search");
            string response = fogBugz.CallRESTAPIFiles(ref logIt, args, null);

            XmlTextReader reader = new XmlTextReader(new System.IO.StringReader(response));
            XPathDocument doc = new XPathDocument(reader);
            XPathNavigator nav = doc.CreateNavigator();
            XPathNodeIterator nodeIterator = (XPathNodeIterator)nav.Evaluate("response/cases/case");
            foreach (XPathNavigator node in nodeIterator)
            {
                if (!node.HasChildren)
                    continue;

                // Iterate through the child nodes
                node.MoveToFirstChild();
                do
                {
                    switch (node.Name)
                    {
                        case "ixStatus":
                            active = (node.Value == "1" ? true : false);
                            break;
                        case "sProject":
                            project = node.Value;
                            break;
                        case "sLatestTextSummary":
                            lastUpdate = node.Value;
                            break;
                    }
                }
                while (node.MoveToNext());
            }
        }
    }
}
