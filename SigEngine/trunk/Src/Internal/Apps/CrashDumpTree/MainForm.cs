using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Text.RegularExpressions;
using System.Collections;
using System.Diagnostics;

namespace CrashDumpTree
{
	public partial class MainForm : Form
	{
		const string cDumpPath = @"\\shares\Shared\QA\Crash_Debug";
		static string GetBetweenParens(string s)
		{
			string o = s.Remove(s.LastIndexOf(')'));
			o = o.Remove(0,o.IndexOf('(')+1);
			return o;
		}
		class MyTreeNode : TreeNode
		{
			public string CallStack;
			public List<string> Dumps = new List<string>();
			public MyTreeNode(string cs)
			{
				var csLines = cs.Split('\n');
				Func<string, bool> lineContainsCheck = s => s.Contains("Crash occurred at");

				Text = csLines
						.FirstOrDefault(s => !lineContainsCheck(s) && s.Trim().Length > 0);
				if (Text == "")
					Text = "NO CALLSTACK";
				Dumps.AddRange(csLines
						.Where(lineContainsCheck)
						.Select(GetBetweenParens));
				CallStack = csLines
								.Where(s => !lineContainsCheck(s) && s.Trim().Length > 0)
								.Aggregate("", (s1, s2) => s1 + '\n' + s2);
			}
		};
		class MyNodeComparer : IComparer
		{
			public int Compare(object a, object b)
			{
				return -(a as TreeNode).Text.CompareTo((b as TreeNode).Text);
			}
		}
		public MainForm()
		{
			InitializeComponent();
			foreach (var dir in Directory.EnumerateDirectories(cDumpPath))
			{
				var node = treeView1.Nodes.Add(dir);

				var callstacks = File.ReadAllText(dir + @"\CallStackLog.txt")
					.Split(new string[] { "END OF CALL STACK" }, StringSplitOptions.None);

				foreach (string cs in callstacks)
				{
					if (cs.Trim().Length <= 0)
						continue;
					var botNode = new MyTreeNode(cs);
					var foundNode = node.Nodes.Cast<MyTreeNode>().FirstOrDefault(b=>b.CallStack==botNode.CallStack) as MyTreeNode;
					if (foundNode != null)
						foundNode.Dumps.AddRange(botNode.Dumps);
					else
						node.Nodes.Add(botNode);
				}
				treeView1.TreeViewNodeSorter = new MyNodeComparer();
				treeView1.Sort();
			}
		}

		private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
		{
			var t = e.Node as MyTreeNode;
			if (t != null)
			{
				textBox1.Text = t.CallStack;
				listBox1.Items.Clear();
				listBox1.Items.AddRange(t.Dumps.ToArray());
			}
		}

		private void treeView1_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
		{
			if (e.Node as MyTreeNode == null)
				Process.Start(e.Node.Text);
		}

		private void listBox1_MouseDoubleClick(object sender, MouseEventArgs e)
		{
			int index = listBox1.IndexFromPoint(e.Location);
			if (index != ListBox.NoMatches)
				Process.Start(listBox1.Items[index].ToString());
		}
	}
}
