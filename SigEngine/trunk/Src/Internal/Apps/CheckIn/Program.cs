using System;
using System.Windows.Forms;

namespace CheckIn
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            try
            {
                Application.Run(new CheckInForm());
            }
            catch (Exception e)
            {
                HandleException(e, true);
            }
        }

        static public void HandleException(Exception e, bool exit)
        {
            string message = e.Message;
            if (e.InnerException != null)
            {
                message += "\n\nInner Exceptions:\n\n";

                Exception innerE = e.InnerException;
                while (innerE != null)
                {
                    message += "\n\n------------------------------";
                    message += innerE.Message;

                    innerE = innerE.InnerException;
                }
            }
            if (e.StackTrace != null)
                message += "\n\nStack Trace:\n" + e.StackTrace;

            MessageBox.Show(message);

            if (exit)
                Application.Exit();
        }

        static public void HandleException(string message, bool exit)
        {
            HandleException(new Exception(message), exit);
        }
    }
}
