using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace CodePreproc.RttiGen
{

    class tParser
    {

        static public string mAccessSpecifierRule = @"(?<access>public|protected|private)([:]*)";
        static public string mClassNameRule = @"(?<className>((::)*\w((::)\w)*)+\b(<\s*([A-Za-z0-9_:]\s*(<.>)*\s*[,]*\s*)+>)*)";
        static public string mTemplateDeclRule = @"(?<template>\btemplate\s*<([^\)>]*)\s*>)";
        static public string mExportRule = @"((?<export>base_export|tools_export|dll_export)\s+)*";

        static public Regex mRegExWhiteSpace =
            new Regex(@"\s*", RegexOptions.Compiled);
        static public Regex mRegExCComment = 
            new Regex(@"/\*[^*]*\*+([^/*][^*]*\*+)*/", RegexOptions.Compiled);
        static public Regex mRegExCppComment = 
            new Regex(@"\s*//.*", RegexOptions.Compiled);
        static public Regex mRegExDeclareReflective =
            new Regex(@"\b(declare_reflector)\b", RegexOptions.Compiled);
        static public Regex mRegExNamespace =
            new Regex(@"(?<namespace>namespace)\s+", RegexOptions.Compiled);
        static public Regex mRegExClassType =
            new Regex(@"(?<type>struct|class)\s+", RegexOptions.Compiled);
        static public Regex mRegExAnyType =
            new Regex(@"\b(struct|class|enum|union)\b", RegexOptions.Compiled);
        static public Regex mRegExUnionType =
            new Regex(@"(?<type>union)\s+", RegexOptions.Compiled);
        static public Regex mRegExEnumType =
            new Regex(@"(?<type>enum)\s+", RegexOptions.Compiled);
        static public Regex mRegExName =
            new Regex( mExportRule + @"(?<name>\w+)", RegexOptions.Compiled);
        static public Regex mRegExConst =
            new Regex(@"\bconst\b", RegexOptions.Compiled);
        static public Regex mRegExMutable =
            new Regex(@"\bmutable\b", RegexOptions.Compiled);
        static public Regex mRegExStatic =
            new Regex(@"\bstatic\b", RegexOptions.Compiled);
        static public Regex mRegExFriend =
            new Regex(@"\bfriend\b", RegexOptions.Compiled);
        static public Regex mRegExArrayDef =
            new Regex(@"\[\s*\w+\s*]", RegexOptions.Compiled);
        static public Regex mRegExBlockStart =
            new Regex(@"{", RegexOptions.Compiled);

        static public Regex mRegExAccessSpecifier =
            new Regex(mAccessSpecifierRule, RegexOptions.Compiled);

        static public Regex mRegExBaseClass =
            new Regex(@"(" + mAccessSpecifierRule + @"\s+)*" + mClassNameRule, RegexOptions.Compiled);

        static public Regex mRegExTemplateParameter =
            new Regex(@"(?<typename>\w+)\s+(?<parameter>\w+)(?<defaultexpr>\s*=\s*\w+)?", RegexOptions.Compiled);
        static public Regex mRegExTemplateDecl =
            new Regex(mTemplateDeclRule, RegexOptions.Compiled);
        static public Regex mRegExClassDecl =
            new Regex(mTemplateDeclRule + @"*\s*(?<type>struct|class){1}\s+" + mExportRule + @"(?<namespace>(::)?(\w+::)*)(?<name>\w+)(\s*:(?!:)\s*(?<parents>[^;{]+))?\s*(?<openbrace>[{])", RegexOptions.Compiled);
        static public Regex mRegExNamespaceDecl =
            new Regex(@"(?<namespace>namespace)\s+(?<name>\w+)\s+(?<openbrace>[{])", RegexOptions.Compiled);
        static public Regex mRegExUnionDecl =
            new Regex(@"(?<union>union)\s+(?<name>\w+)\s+(?<openbrace>[{])", RegexOptions.Compiled);
        static public Regex mRegExEnumDecl =
            new Regex(@"(?<enum>enum)\s+(?<name>\w+)\s+(?<openbrace>[{])", RegexOptions.Compiled);
        static public Regex mRegExTypedef =
            new Regex(@"typedef\s*[^;]+;", RegexOptions.Compiled);
        static public Regex mRegExFunctionPointer =
            new Regex(@"\([^\*]*\*[^\)]+\)\s*\([^\)]*\)", RegexOptions.Compiled);

        //static public Regex mRegExFunction =
        //    new Regex(@"(?<method>[\w~][^;\(]+)\s*\((?<params>[^\)]*)\)\s*(?<qualifiers>[^\]{;]*)(?<decltype>[{;])", RegexOptions.Compiled);
        static public Regex mRegExFunction =
            new Regex(@"(?<return>\w*([&\*])*)\s+(?<method>[\w~][^;\(]+)\s*\((?<params>[^\)]*)\)\s*(?<qualifiers>[^\]{;]*)(?<decltype>[{;])", RegexOptions.Compiled);

        static public Regex mRegExClassMember =
            new Regex(@"(?<member>\w[^{;\(.]+);", RegexOptions.Compiled);
        static public Regex mRegExVarDeclPiece =
            new Regex(@"(\w+<([^\)]*)\s*>\s*)|(([*]*)\s*((::)*\w((::)\w)*)+\b\s*(\[\s*\w*\s*\])*\s*[;,]*)", RegexOptions.Compiled);

        // when adding new modifiers, be sure to include them in the array mAllowedModifierMacros
        static public Regex mRegExDynamicArrayCount =
            new Regex(@"dynamic_array_count[(]\s*(?<name>\w+)\s*[)]", RegexOptions.Compiled);
        static public Regex[] mAllowedModifierMacros = new Regex[] { mRegExDynamicArrayCount };

        static public int fFindFirstOpenBraceAbove(string text, int pos)
        {
            return fFindFirstOpenBraceAbove(text, pos, "{", "}");
        }

        static public int fFindFirstOpenBraceAbove(string text, int pos, string openBrace, string closeBrace)
        {
            // assumes comments have been removed
            if (pos < 0)
                return -1;

            Stack<int> braceStack = new Stack<int>();
            string subText = text.Substring(pos);
            for (; !subText.StartsWith(openBrace) || braceStack.Count > 0; subText = text.Substring(pos))
            {
                if (subText.StartsWith(closeBrace))
                    braceStack.Push(text[pos]);
                else if (subText.StartsWith(openBrace))
                    braceStack.Pop();
                --pos;
                if (pos < 0)
                    break;
            }

            if (subText.StartsWith(openBrace))
                return pos;
            return -1;
        }

        static public int fFindClosingBrace(string text, int pos)
        {
            return fFindClosingBrace(text, pos, "{", "}");
        }

        static public int fFindClosingBrace(string text, int openBracePos, string openBrace, string closeBrace)
        {
            int pos = openBracePos + 1;

            if (pos >= text.Length)
                return -1;

            Stack<int> braceStack = new Stack<int>();
            string subText = text.Substring(pos);
            for (; !subText.StartsWith(closeBrace) || braceStack.Count > 0; subText = text.Substring(pos))
            {
                if (subText.StartsWith(openBrace))
                    braceStack.Push(text[pos]);
                else if (subText.StartsWith(closeBrace))
                    braceStack.Pop();
                ++pos;
                if (pos >= text.Length)
                    break;
            }

            if (subText.StartsWith(closeBrace))
                return pos;
            return -1;
        }

        public delegate bool tHandleMatchDelegate(Regex re, ref Match match, ref string text);

        public void fForEachMatch(Regex re, ref string text, tHandleMatchDelegate callback)
        {
            Match match = re.Match(text);
            while (match.Success)
            {
                if (!callback(re, ref match, ref text))
                    break;
            }
        }

        public delegate string tHandleBlockDelegate(string tagAndName, string body);

        public string fParseBlock(string text, Regex re, tHandleBlockDelegate handler)
        {
            Match match = re.Match(text);

            while (match.Success)
            {
                // strip the namespace tag and name
                int tagStart = match.Index;
                int openBrace = match.Index + match.Length - 1;
                int closeBrace = fFindClosingBrace(text, openBrace);
                string tagAndName = text.Substring(match.Index, match.Length);
                string body = text.Substring(openBrace, closeBrace - openBrace + 1);

                // create the child namespace
                string replacement = handler(tagAndName, body);

                // remove all namespace text from my current body
                string startText = text.Substring(0, tagStart);
                string endText = text.Substring(closeBrace + 1, text.Length - closeBrace - 1);

                //text = text.Remove(tagStart, closeBrace - tagStart + 1);
                text = startText + replacement + endText;

                // find next namespace match
                match = re.Match(text);
            }

            return text;
        }

    }

}
