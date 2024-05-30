using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace FindAndFixMissingLocStrings
{
	class Program
	{
		static readonly string cLocFolder = Environment.GetEnvironmentVariable("sigcurrentproject") + @"\res\Loc\";
		const string cLocFile = @"\text.locml";
		const string cNameId = "name=\"";
		static readonly string cSrcFile = cLocFolder + "en" + cLocFile;
		static readonly string[] cFIGSJ = new string[] { "fr", "it", "de", "es", "ja" };

		static string ExtractName(string line)
		{
			string name = line.Remove(0, line.IndexOf(cNameId) + cNameId.Length);
			int quote = name.IndexOf("\"");
			name = name.Remove(quote, name.Length - quote);
			return name;
		}

		static int IndexOfInnerString(List<string> list, string arg)
		{
			for (int i = 0; i < list.Count; ++i)
			{
				if (list[i].Contains(arg))
					return i;
			}
			return -1;
		}

		static void Main(string[] args)
		{
			string[] srcLines = File.ReadAllLines(cSrcFile);
			foreach (var lang in cFIGSJ)
			{
				string destFileName = cLocFolder + lang + cLocFile;
				List<string> destLines = File.ReadAllLines(destFileName).ToList();
				int indexOfLastGoodName = -1;
				for (int i = 0; i < srcLines.Length; ++i)
				{
					if(!srcLines[i].Contains(cNameId))
						continue;

					string name = ExtractName(srcLines[i]);
					int insertIndex = IndexOfInnerString(destLines, name);
					if (insertIndex == -1)
					{
						Debug.Assert(indexOfLastGoodName != -1);
						insertIndex = indexOfLastGoodName + 1;
						destLines.Insert(insertIndex, srcLines[i]);
					}
					indexOfLastGoodName = insertIndex;
				}
				File.WriteAllLines(destFileName, destLines);
			}
		}
	}
}
