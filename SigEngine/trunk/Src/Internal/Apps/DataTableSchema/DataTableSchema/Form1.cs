using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using Excel = Microsoft.Office.Interop.Excel;
using System.Xml;
using System.Xml.Serialization;

namespace DataTableSchema
{

    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        tDataTableSchema mLoadedSchema;

        private void bRefresh_Click(object sender, EventArgs e)
        {
            string resDirectory = Environment.GetEnvironmentVariable("SigCurrentProject") + "\\Res\\";
            mFileList.Items.Clear();

            fAddResItems(resDirectory);
        }

        private void fAddResItems(string resDirectory)
        {
            List<string> results = new List<string>( );
            results.AddRange(Directory.GetFiles(resDirectory, "*.xlsx", SearchOption.AllDirectories));

            for (int i = 0; i < results.Count; ++i)
                mFileList.Items.Add(results[i]);
        }

        private void bApply_Click(object sender, EventArgs e)
        {
            if (mLoadedSchema != null)
            {
                for (int i = 0; i < mFileList.SelectedItems.Count; ++i)
                {
                    string filename = mFileList.SelectedItems[i].ToString();
                    tDataTable table = new tDataTable();
                    table.fParseTable(filename);
                    table.fUpdateForSchema(mLoadedSchema);
                    table.fSaveTable(filename, filename);
                }
            }
        }

        private void bLoadSchema_Click(object sender, EventArgs e)
        {
            mLoadedSchema = tDataTableSchema.fLoad("c:\\testschema.txt");
        }

        private void bSaveSchema_Click(object sender, EventArgs e)
        {
            if (mFileList.SelectedItems.Count > 0)
            {
                tDataTable table = new tDataTable();
                table.fParseTable(mFileList.SelectedItems[0].ToString( ));
                mLoadedSchema = table.fMakeSchema( );
                mLoadedSchema.fSave( "C:\\testschema.txt" );
            }
        }
    }


    class tExcelSingleton
    {
        static tExcelSingleton mInst = new tExcelSingleton();
        static public tExcelSingleton fInstance() { return mInst; }
        
        public Excel.Application mExcelApp;

        private tExcelSingleton() 
        {
            mExcelApp = new Excel.Application();
            mExcelApp.ScreenUpdating = false;
        }
    }


    public class tDataTableSchema
    {
        public class tColumn
        {
            [XmlAttribute("Name")]
            public string mName;
            [XmlAttribute("Default")]
            public string mDefaultValue = "";
        }
        public class tTable
        {
            [XmlAttribute("Name")]
            public string mName;
            public List<tColumn> Columns = new List<tColumn>();
        }
        public List<tTable> Tables = new List<tTable>();
        
        public void fSave(string filepath)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(tDataTableSchema));
            TextWriter textWriter = new StreamWriter(filepath);
            serializer.Serialize(textWriter, this);
            textWriter.Close();
        }

        static public tDataTableSchema fLoad(string filepath)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(tDataTableSchema));
            XmlReader textReader = new XmlTextReader(filepath);

            tDataTableSchema dts = (tDataTableSchema)serializer.Deserialize(textReader);
            textReader.Close();

            return dts;
        }

        public tTable fFindTable(string name)
        {
            return Tables.Find(delegate(tTable t) { return t.mName == name; });
        }
    }

    public class tDataTable
    {
        public class tColumn
        {
            public string mName;
            public List<string> mRowValues = new List<string>( );

            public tColumn(string name )
            {
                mName = name;
            }

            public void fFillRows(int count, string value)
            {
                mRowValues.Clear();
                for (int i = 0; i < count; ++i)
                    mRowValues.Add(value);
            }
        }

        public class tTable
        {
            public string mName;
            public int mStartX;
            public int mStartY;
            public List<tColumn> mColumns = new List<tColumn>( );

            public tTable(string name, int startX, int startY)
            {
                mName = name;
                mStartX = startX;
                mStartY = startY;
            }

            public tColumn fFindColumn(string name)
            {
                return mColumns.Find(delegate(tColumn t) { return t.mName == name; });
            }

            public int fRowCount()
            {
                return (mColumns.Count > 0) ? mColumns[0].mRowValues.Count : 0;
            }
        }
        public List<tTable> mTables = new List<tTable>();


        public void fWarning(String msg)
        {
            MessageBox.Show(msg);
        }

        public tTable fFindTable(string name)
        {
            return mTables.Find(delegate(tTable t) { return t.mName == name; });
        }

        public void fParseTable(string filePath)
        {
            mTables.Clear();

            Excel.Workbook workBook = tExcelSingleton.fInstance().mExcelApp.Workbooks.Open(filePath);
            foreach (Excel.Worksheet sheet in workBook.Sheets)
            {
                Excel.Range range = sheet.UsedRange;
                object[,] cells = (object[,])range.Value;

				if( cells == null )
					continue;

                // find tables
                int xMax = cells.GetUpperBound(0);
                int yMax = cells.GetUpperBound(1);
                int xMin = cells.GetLowerBound(0);
                int yMin = cells.GetLowerBound(1);

                for (int x = xMin; x <= xMax; ++x)
                {
                    for (int y = yMin; y <= yMax; ++y)
                    {
                        object cellObj = cells[x, y];
                        if (cellObj == null)
                            continue;

                        string cell = cellObj.ToString();

                        if (cell[0] == '~')
                        {
                            tTable table = new tTable(cell,x,y);
                            mTables.Add(table);

                            // fill tables
                            int col = y;

                            // columns
                            string colName = "";
                            bool found = false;
                            do 
                            {
                                if (col > yMax)
                                    break;

                                object colCell = cells[x, col];
                                if (colCell == null)
                                    break;

                                colName = colCell.ToString();
                                found = (colName.Contains( ':' ) || colName.Contains( '~' ));
                                if (found)
                                {
                                    table.mColumns.Add(new tColumn(colName));
                                }

                                ++col;
                            } while (found);

                            // rows
                            int row = x + 1;
                            do 
                            {
                                // Check to see if this row has any non blank values.
                                bool rowHadValue = false;
                                List<string> rowValues = new List<string>();
                                for (int i = 0; i < table.mColumns.Count; ++i)
                                {
                                    object rowCell = cells[row, y + i];
                                    string rowVal = (rowCell != null) ? rowCell.ToString() : "";

                                    if (rowVal.Length > 0)
                                        rowHadValue = true;

                                    rowValues.Add(rowVal);
                                }

                                // if we had any, store them all. even blanks
                                if (rowHadValue)
                                {
                                    for (int i = 0; i < table.mColumns.Count; ++i)
                                        table.mColumns[i].mRowValues.Add(rowValues[i]);
                                }
                                else
                                    break;

                                ++row;
                            } while (row <= xMax);

                        }
                    }
                }

                workBook.Close();
            }
        }
        
        public void fSaveTable(string filePath, string formatPath)
        {
            Excel.Workbook workBook = tExcelSingleton.fInstance().mExcelApp.Workbooks.Add();

            Excel.Worksheet sheet = workBook.Sheets.Add();

            //// paste in formats
            //Excel.Workbook formatWorkBook = tExcelSingleton.fInstance().mExcelApp.Workbooks.Open(formatPath);
            //foreach (Excel.Worksheet s in formatWorkBook.Sheets)
            //{
            //    s.Copy();
            //    sheet.PasteSpecial(Excel.XlPasteType.xlPasteFormats);
            //    break;
            //}
            //formatWorkBook.Close(false);

            foreach (tTable t in mTables)
            {
                for (int i = 0; i < t.mColumns.Count; ++i)
                {
                    int y = t.mStartY + i;
                    sheet.Cells[t.mStartX, y] = t.mColumns[i].mName;

                    for (int r = 0; r < t.mColumns[i].mRowValues.Count; ++r)
                        sheet.Cells[t.mStartX + r + 1, y] = t.mColumns[i].mRowValues[r];
                }
            }

            workBook.SaveAs(filePath);
            workBook.Close(false);
        }

        public tDataTableSchema fMakeSchema()
        {
            tDataTableSchema schema = new tDataTableSchema();

            foreach (tTable t in mTables)
            {
                tDataTableSchema.tTable nt = new tDataTableSchema.tTable();
                schema.Tables.Add(nt);

                nt.mName = t.mName;

                foreach (tColumn c in t.mColumns)
                {
                    tDataTableSchema.tColumn nc = new tDataTableSchema.tColumn();
                    nt.Columns.Add(nc);

                    nc.mName = c.mName;
                }
            }

            return schema;
        }

        //class tSchemaTableChange
        //{
        //    public string mTableName;
        //    List<tDataTableSchema.tColumn> mColumnsToAdd = new List<tDataTableSchema.tColumn>( );
        //}

        public void fUpdateForSchema(tDataTableSchema newS)
        {
            tDataTableSchema myS = fMakeSchema();

            // for each table i have.
            //  find it in the new schema.
            //  update my table to match it.
            for (int i = 0; i < myS.Tables.Count; ++i)
            {
                string tableName = myS.Tables[i].mName;
                tDataTableSchema.tTable newT = newS.fFindTable(tableName);
                if (newT == null)
                    fWarning("Couldn't find table in new schema: '" + tableName + "'");
                else
                {
                    tTable dt = fFindTable(tableName);
                    if (dt == null)
                        fWarning("wtf, my table didnt exist?");
                    {
                        List<tColumn> newColumns = new List<tColumn>();

                        foreach( tDataTableSchema.tColumn c in newT.Columns )
                        {
                            tColumn myCol = dt.fFindColumn( c.mName );
                            if (myCol == null)
                            {
                                myCol = new tColumn(c.mName);
                                myCol.fFillRows(dt.fRowCount(), c.mDefaultValue);

                                fWarning("Added: '" + c.mName + "' to table: '" + tableName + "'");
                            }
                            
                            newColumns.Add(myCol);
                        }

                        dt.mColumns = newColumns;
                    }
                }
            }
        }
    }
}
