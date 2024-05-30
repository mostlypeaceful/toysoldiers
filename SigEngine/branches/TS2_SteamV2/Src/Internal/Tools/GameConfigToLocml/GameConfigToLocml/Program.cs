using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml;
using System.Xml.Linq;

namespace GameConfigToLocml
{
    class Program
    {
        static Dictionary<string, XDocument> locml = new Dictionary<string, XDocument>();
        static Dictionary<string, string> localeToName = new Dictionary<string, string>();

        static XDocument FindLocml(string language)
        {
            XDocument doc = null;
            if (locml.TryGetValue(language, out doc))
                return doc;
            // Didn't find it, so create it
            doc = new XDocument();
            doc.Add(new XElement("Locml"));
            ((XElement)doc.FirstNode).Add(new XElement("StringTable"));
            locml.Add(language, doc);
            return doc;
        }

        static void AddString(string language, string key, string text)
        {
            var doc = FindLocml(language);
            var i = new XElement("i");
            i.SetAttributeValue("name", key);
            var t = new XElement("Text");
            t.Value = text;
            i.Add(t);
            ((XElement)((XElement)doc.FirstNode).FirstNode).Add(i);
        }

        static void Main(string[] args)
        {
            localeToName["en-US"] = "english";
            localeToName["fr-FR"] = "french";
            localeToName["it-IT"] = "italian";
            localeToName["de-DE"] = "german";
            localeToName["es-ES"] = "spanish";
            localeToName["ja-JP"] = "japanese";
            localeToName["ko-KR"] = "korean";
            localeToName["zh-CHT"] = "tchinese";

            XDocument src = XDocument.Load(args[0]);

            var name = XNamespace.Get("http://www.xboxlive.com/xlast");
            var strings = from n in src.Element(name + "XboxLiveSubmissionProject").Element(name + "GameConfigProject").Element(name + "LocalizedStrings").Elements()
                          where n.Name == name + "LocalizedString"
                          select n;

            foreach (var str in strings)
            {
                var key = str.Attribute("friendlyName").Value;
                foreach (var t in str.Elements(name + "Translation"))
                {
                    var language = t.Attribute("locale").Value.Substring(0, 2);
                    var text = t.Value;
                    AddString(language, key, text);
                }
            }

            foreach (var doc in locml)
            {
                var filename = doc.Key + "_gameconfig.locml";
                doc.Value.Save(filename);
            }

            // Create loc vdf for upload to Steam
            var achievements = from a in src.Element(name + "XboxLiveSubmissionProject").Element(name + "GameConfigProject").Element(name + "Achievements").Elements()
                          where a.Name == name + "Achievement"
                          select a;

            using (var writer = new StreamWriter("262120_loc_all.vdf", false, System.Text.Encoding.UTF8))
            {
                writer.WriteLine("\"lang\"");
                writer.WriteLine("{");
                foreach (var lang in localeToName)
                {
                    writer.WriteLine("\t\"" + lang.Value + "\"");
                    writer.WriteLine("\t{");
                    writer.WriteLine("\t\t\"Tokens\"");
                    writer.WriteLine("\t\t{");
                    foreach (var ach in achievements)
                    {
                        writer.Write("\t\t\t\"NEW_ACHIEVEMENT_1_" + (int.Parse(ach.Attribute("id").Value) - 1) + "_NAME\"\t\"");
                        var achName = (from n in strings
                                      where n.Attribute("id").Value == ach.Attribute("titleStringId").Value
                                      select (from t in n.Elements()
                                             where t.Name == name + "Translation" && t.Attribute("locale").Value == lang.Key
                                             select t.Value).FirstOrDefault()).FirstOrDefault();
                        writer.WriteLine(achName + "\"");
                        writer.Write("\t\t\t\"NEW_ACHIEVEMENT_1_" + (int.Parse(ach.Attribute("id").Value) - 1) + "_DESC\"\t\"");
                        var achDesc = (from n in strings
                                      where n.Attribute("id").Value == ach.Attribute("unachievedStringId").Value
                                      select (from t in n.Elements()
                                             where t.Name == name + "Translation" && t.Attribute("locale").Value == lang.Key
                                             select t.Value).FirstOrDefault()).FirstOrDefault();
                        writer.WriteLine(achDesc + "\"");
                    }
                    writer.WriteLine("\t\t}");
                    writer.WriteLine("\t}");
                }
                writer.WriteLine("}");
            }
        }
    }
}
