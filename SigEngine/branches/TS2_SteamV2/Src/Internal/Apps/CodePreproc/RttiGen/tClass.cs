using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace CodePreproc.RttiGen
{
    class tClass : tParser
    {
        public class tBase
        {
            public string mAccess = "";
            public string mName = "";

            public tBase(string access, string name)
            {
                if (access != null && access.Length > 0)
                    mAccess = access;
                else
                    mAccess = "private";

                mName = name;
            }
        }

        public class tMemberVariable
        {
            public string mType;
            public string mName;
            public string mCount; // for arrays
            public string mDynamicArrayCount; // the name of the variable determining the length of the dynamic array (if any)

            public tMemberVariable(string type, string name, string count, string dynArrayCount)
            {
                mType = type;
                mName = name;
                mCount = count;
                mDynamicArrayCount = dynArrayCount;
            }
        }

        public bool mIsReflective = false;
        string mClassType = "";
        string mClassName = "";
        string mTemplateParams = "";
        public List<tBase> mBaseTypes = new List<tBase>();
        public List<tMemberVariable> mMemberVariables = new List<tMemberVariable>();
        public List<tClass> mClasses = new List<tClass>();
        public List<tUnion> mUnions = new List<tUnion>();
        public List<tTypedef> mTypedefs = new List<tTypedef>();
        public List<tEnum> mEnums = new List<tEnum>();

        public tClass(string tagAndNameText, string bodyText)
        {
            fParseTagAndName(tagAndNameText);
            fParseBody(bodyText);
        }

        public tClass(string tagAndNameText, string bodyText, string parentClassName)
        {
            fParseTagAndName(tagAndNameText, parentClassName);
            fParseBody(bodyText);
        }

        public string fGetSimpleDeclaration()
        {
            return mClassType + " " + mClassName;
        }

        public bool fHasReflectedClass()
        {
            if (mIsReflective)
                return true;
            for (int i = 0; i < mClasses.Count; ++i)
                if (mClasses[i].fHasReflectedClass())
                    return true;
            return false;
        }

        public bool fHasTemplates()
        {
            if (mTemplateParams.Length > 0 && fHasReflectedClass())
                return true;
            for (int i = 0; i < mClasses.Count; ++i)
                if (mClasses[i].fHasTemplates())
                    return true;
            return false;
        }

        public bool fHasNonTemplates()
        {
            if (mTemplateParams.Length == 0 && fHasReflectedClass())
                return true;
            for (int i = 0; i < mClasses.Count; ++i)
                if (mClasses[i].fHasNonTemplates())
                    return true;
            return false;
        }

        public void fOutput(System.IO.StreamWriter outputFileStream, bool templates)
        {
            if (templates && !fHasTemplates())
                return;
            if (!templates && !fHasNonTemplates())
                return;

            for (int i = 0; i < mClasses.Count; ++i)
                mClasses[i].fOutput(outputFileStream, templates);

            bool writeMe = ((templates && mTemplateParams.Length > 0) || (!templates && mTemplateParams.Length == 0));

            if (writeMe)
            {
                outputFileStream.WriteLine();
                outputFileStream.WriteLine("//");
                outputFileStream.WriteLine("// " + mClassName);

                tOutputVariableNames ovn = fGetOutputVariableNames();

                // order matters here, as it affects the order 
                // of the constructors when we start up at run-time:

                // 1 - FIRST write base classes
                fOutputBaseDescArray(outputFileStream, ovn);
                // 2 - SECOND write member vars
                fOutputMemberDescArray(outputFileStream, ovn);
                // 3 - THIRD write reflector
                fOutputClassReflector(outputFileStream, ovn);
                outputFileStream.WriteLine();
            }
        }

        private string fGetTemplateHeader()
        {
            if (mTemplateParams.Length == 0)
                return "";
            return "template" + mTemplateParams + "\r\n";
        }

        private bool fCleanTemplateParams(Regex re, ref Match match, ref string text)
        {
            Group typenameGroup = match.Groups["typename"];
            text = text.Remove(typenameGroup.Index, typenameGroup.Length).Trim();
            match = re.Match(text);
            return true;
        }

        private string fClassNameWithTemplateParams()
        {
            if (mTemplateParams.Length == 0)
                return mClassName;

            // TODO consider things like <template<class A, class B> class C, int N, typename B>

            string templateParams = mTemplateParams.Trim();
            templateParams = templateParams.Substring(1, templateParams.Length - 2);

            fForEachMatch(mRegExTemplateParameter, ref templateParams, fCleanTemplateParams);

            return mClassName + "< " + templateParams.Trim() + " >";
        }

        class tOutputVariableNames
        {
            public string mTemplateHeader;
            public string mClassNameWithTemplateParams;
            public string mBaseDescArrayType;
            public string mBaseDescArrayName;
            public string mMemberDescArrayType;
            public string mMemberDescArrayName;
            public string mReflectorType;
            public string mReflectorName;
        };

        private tOutputVariableNames fGetOutputVariableNames()
        {
            tOutputVariableNames o = new tOutputVariableNames();

            o.mTemplateHeader = fGetTemplateHeader();
            o.mClassNameWithTemplateParams = fClassNameWithTemplateParams( );
            o.mBaseDescArrayType = "::Sig::Rtti::tBaseClassDesc";
            o.mBaseDescArrayName = o.mClassNameWithTemplateParams + "::gBases";
            o.mMemberDescArrayType = "::Sig::Rtti::tClassMemberDesc";
            o.mMemberDescArrayName = o.mClassNameWithTemplateParams + "::gMembers";
            o.mReflectorType = "::Sig::Rtti::tReflector";
            o.mReflectorName = o.mClassNameWithTemplateParams + "::gReflector";

            return o;
        }

        private string fWrapInQuotes(string s)
        {
            return "\"" + s + "\"";
        }

        private void fOutputBaseClass(tBase baseClass, System.IO.StreamWriter outputFileStream, tOutputVariableNames ovn)
        {
            if (baseClass == null)
            {
                outputFileStream.WriteLine("\t" + "::Sig::Rtti::fDefineBaseClassDesc()");
            }
            else
            {
                outputFileStream.Write("\t" + 
                    "::Sig::Rtti::fDefineBaseClassDesc< " + mClassName + ", " + baseClass.mName + " >( " );
                outputFileStream.Write( fWrapInQuotes(mClassName) + ", " );
                outputFileStream.Write( fWrapInQuotes(baseClass.mName) + ", " );
                outputFileStream.Write( fWrapInQuotes(baseClass.mAccess) );
                outputFileStream.WriteLine( " )," );
            }
        }

        private void fOutputMemberVariable(tMemberVariable memberVar, System.IO.StreamWriter outputFileStream, tOutputVariableNames ovn)
        {
            if (memberVar == null)
            {
                outputFileStream.WriteLine("\t" + "::Sig::Rtti::fDefineClassMemberDesc()");
            }
            else
            {
                outputFileStream.Write("\t" + 
                    "::Sig::Rtti::fDefineClassMemberDesc< " + mClassName + ", " + memberVar.mType + " >( " );
                outputFileStream.Write( fWrapInQuotes(mClassName) + ", " );
                outputFileStream.Write( fWrapInQuotes(memberVar.mType) + ", " );
                outputFileStream.Write( fWrapInQuotes(memberVar.mName) + ", " );
                outputFileStream.Write( fWrapInQuotes("public") + ", " );
                outputFileStream.Write( "offsetof( " + mClassName + ", " + memberVar.mName + " )" + ", " );
                outputFileStream.Write( memberVar.mCount + ", " );

                if (memberVar.mDynamicArrayCount != null && memberVar.mDynamicArrayCount.Length > 0)
                {
                    string args = mClassName + ", " + memberVar.mDynamicArrayCount;
                    outputFileStream.Write("offsetof( " + args + " )" + ", ");
                    outputFileStream.Write("sizeof_member( " + args + " )" );
                }
                else
                {
                    outputFileStream.Write("0" + ", ");
                    outputFileStream.Write("0");
                }

                outputFileStream.WriteLine( " )," );
            }
        }

        private void fOutputBaseDescArray(System.IO.StreamWriter outputFileStream, tOutputVariableNames ovn)
        {
            outputFileStream.WriteLine(
                ovn.mTemplateHeader +
                "const " + ovn.mBaseDescArrayType + " " + ovn.mBaseDescArrayName + "[]={");

            for (int i = 0; i < mBaseTypes.Count; ++i)
                fOutputBaseClass(mBaseTypes[i], outputFileStream, ovn);
            fOutputBaseClass(null, outputFileStream, ovn);

            outputFileStream.WriteLine("};");
        }

        private void fOutputMemberDescArray(System.IO.StreamWriter outputFileStream, tOutputVariableNames ovn)
        {
            outputFileStream.WriteLine(
                ovn.mTemplateHeader +
                "const " + ovn.mMemberDescArrayType + " " + ovn.mMemberDescArrayName + "[]={");

            for (int i = 0; i < mMemberVariables.Count; ++i)
                fOutputMemberVariable(mMemberVariables[i], outputFileStream, ovn);
            fOutputMemberVariable(null, outputFileStream, ovn);

            outputFileStream.WriteLine("};");
        }

        private void fOutputClassReflector(System.IO.StreamWriter outputFileStream, tOutputVariableNames ovn)
        {
            string reflectorDef = ovn.mTemplateHeader + "const " + ovn.mReflectorType + " " + ovn.mReflectorName + " = ";
            string reflectorConstructionParams = "::Sig::Rtti::fDefineReflector< " + mClassName + " >( " +
                fWrapInQuotes(mClassName) + ", " +
                ovn.mBaseDescArrayName + ", " + 
                ovn.mMemberDescArrayName + " );";

            outputFileStream.WriteLine(reflectorDef + reflectorConstructionParams);
        }

        private bool fAddBaseClass(Regex re, ref Match match, ref string text)
        {
            string s = match.Value;

            // get the access specifier, if any (defaults to private)
            string access = mRegExAccessSpecifier.Match(s).Value;
            if (access.Length > 0)
                s = s.Replace(access, "").Trim();
            if (s.Length == 0)
                throw new System.Exception("invalid class declaration");

            mBaseTypes.Add(new tBase(access, s));
            match = match.NextMatch();
            return true;
        }

        private void fParseTagAndName(string text)
        {
            fParseTagAndName(text, "");
        }

        private void fParseTagAndName(string text, string parentClassName)
        {
            Match match;

            // first check for template parameters
            match = mRegExTemplateDecl.Match(text);
            if (match.Success)
            {
                mTemplateParams = match.Value;
                text = text.Remove(match.Index, match.Length).Trim();
                mTemplateParams = mTemplateParams.Substring(mTemplateParams.IndexOf('<')).Trim();
            }

            // store the name for which "type" of class (i.e., struct or class)
            match = mRegExClassType.Match(text);
            if (!match.Success)
                throw new System.Exception("invalid class declaration");

            mClassType = text.Substring(match.Index, match.Length).Trim();

            // store the actual name of the class (i.e., this is the c++ type)
            if (parentClassName != null && parentClassName.Length > 0)
                mClassName = parentClassName + "::";
            else
                mClassName = "";
            string next = text.Substring(match.Index + match.Length);
            match = mRegExName.Match(next);
            if (!match.Success)
                mClassName = "Anonymous";
            else
                mClassName += match.Groups["name"].Value.Trim();

            mClassName = mClassName.Replace(">>", "> >");

            next = next.Substring(match.Index + match.Length).Trim();

            // extract base classes (handles optional access specifier, as well as types with template params)
            fForEachMatch(mRegExBaseClass, ref next, fAddBaseClass);
        }

        private string fAddUnion(string tagAndName, string body)
        {
            tUnion cl = new tUnion(tagAndName, body);
            mUnions.Add(cl);
            return cl.fGetSimpleDeclaration();
        }

        private string fAddEnum(string tagAndName, string body)
        {
            tEnum cl = new tEnum(tagAndName, body);
            mEnums.Add(cl);
            return cl.fGetSimpleDeclaration();
        }

        private string fAddClass(string tagAndName, string body)
        {
            tClass cl = new tClass(tagAndName, body, mClassName);
            mClasses.Add(cl);
            return cl.fGetSimpleDeclaration();
        }

        private string fRemoveBlock(string tagAndName, string body)
        {
            return ";";
        }

        private bool fParseAccessSpecifiers(Regex re, ref Match match, ref string text)
        {
            text = text.Remove(match.Index, match.Length);
            match = re.Match(text);
            return true;
        }

        private bool fParseTypedef(Regex re, ref Match match, ref string text)
        {
            mTypedefs.Add(new tTypedef(match.Value));
            text = text.Remove(match.Index, match.Length);
            match = re.Match(text);
            return true;
        }

        private bool fParseFunction(Regex re, ref Match match, ref string text)
        {
            // skip allowed modifier macros that look like functions
            for (int i = 0; i < mAllowedModifierMacros.Length; ++i)
            {
                if (mAllowedModifierMacros[i].Match(match.Value).Success)
                {
                    match = match.NextMatch();
                    return true;
                }
            }

            // TODO currently we're picking up function pointers in here as well;
            // just make this method determines which it is, and handle accordingly
            // TODO store function?
            text = text.Remove(match.Index, match.Length);
            match = re.Match(text);
            return true;
        }

        private string fFindDynamicArrayCountOption(ref string text)
        {
            int startIndex = 0;
            string search = text.Substring(startIndex);
            int indexOfNewline = search.IndexOfAny(new char[] { '\n', '\r' });
            if (indexOfNewline < 0 || indexOfNewline >= search.Length)
                return "";

            search = search.Remove(indexOfNewline);

            Match match = mRegExDynamicArrayCount.Match(search);
            if (match.Success)
            {
                string dynamicArrayCountName = match.Groups["name"].Value.Trim();
                text = text.Remove(startIndex, indexOfNewline);
                return dynamicArrayCountName;
            }

            return "";
        }

        private bool fParseMemberVariable(Regex re, ref Match match, ref string text)
        {
            string decl = match.Value;

            if (mRegExStatic.Match(decl).Success || mRegExFriend.Match(decl).Success)
            {
                // this is a static variable or a friend declaration, doesn't count
                match = match.NextMatch();
                return true;
            }

            // clean any 'const' specifiers, we don't care about that
            decl = mRegExConst.Replace(decl, "").Trim();

            // remove current member variable text
            text = text.Substring(match.Index + match.Length).Trim();

            // search for options
            string dynamicArrayCount = fFindDynamicArrayCountOption(ref text);
            // TODO grab any extra text at the end of the line for meta-data

            // match all the pieces from this declaration, and separate them
            Match m = mRegExVarDeclPiece.Match(decl);
            List<string> pieces = new List<string>();
            while (m.Success)
            {
                string piece = m.Value;
                if (piece.Length > 0 && (piece[piece.Length-1] == ';' || piece[piece.Length-1] == ','))
                    piece = piece.Substring(0, m.Value.Length-1);

                // eat all white space
                piece = mRegExWhiteSpace.Replace(piece, "");

                // check if text is a type
                if (!mRegExAnyType.Match(piece).Success)
                    pieces.Add(piece);

                m = m.NextMatch();
            }

            // now we're actually ready to add the individual member declarations;
            // it is assumed now that the type is in the first slot
            for (int i = 1; i < pieces.Count; ++i)
            {
                string type = pieces[0].Replace(">>", "> >");
                string name = pieces[i];
                string count = "1";

                // handle pointers
                while (name.Length > 0 && name[0] == '*')
                {
                    type += '*';
                    name = name.Substring(1);
                }

                // handle arrays (i.e., char buf[256])
                Match arrayBrackets = mRegExArrayDef.Match(name);
                if (arrayBrackets.Success)
                {
                    count = arrayBrackets.Value.Substring(1,arrayBrackets.Value.Length-2).Trim();
                    name = name.Substring(0, arrayBrackets.Index).Trim();
                }

                mMemberVariables.Add(new tMemberVariable(type, name, count, dynamicArrayCount));
            }

            match = re.Match(text);
            return true;
        }

        private void fParseBody(string text)
        {
            text = fParseBlock(text, mRegExClassDecl, fAddClass);
            text = fParseBlock(text, mRegExUnionDecl, fAddUnion);
            text = fParseBlock(text, mRegExEnumDecl, fAddEnum);

            if (text.Length > 0 && text[0] == '{' && text[text.Length-1] == '}')
                text = text.Substring(1, text.Length-2).Trim();
            text = fParseBlock(text, mRegExBlockStart, fRemoveBlock);

            // parse for whether this class is reflective
            mIsReflective = mRegExDeclareReflective.Match(text).Success;

            if (!mIsReflective)
                return; // skip non-reflective classes

            // handle access specifiers
            fForEachMatch(mRegExAccessSpecifier, ref text, fParseAccessSpecifiers);

            // handle typedefs
            fForEachMatch(mRegExTypedef, ref text, fParseTypedef);

            // handle functions and function pointers
            fForEachMatch(mRegExFunction, ref text, fParseFunction);

            // finally, find normal data members
            fForEachMatch(mRegExClassMember, ref text, fParseMemberVariable);
        }
    
    }
}
