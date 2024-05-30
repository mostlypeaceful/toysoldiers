using System;

namespace DownResRes
{
	class ConsoleProgressListener : IProgressListener
	{
		static void WithForeground( ConsoleColor fg, Action a )
		{
			var oldFG = Console.ForegroundColor;
			Console.ForegroundColor = fg;
			a();
			Console.ForegroundColor = oldFG;
		}

		public void On( StartProcessingEvent e )
		{
			Console.Write( "\nSimplifying texture {0} of {1}: {2}...", e.FileNumber, e.FilesCount, e.File );
		}

		public void On( ErrorProcessingEvent e )
		{
			WithForeground( ConsoleColor.Red, () => Console.Write( " Error: {0}", e.Description ) );
		}

		public void On( InfoProcessingEvent e )
		{
			Console.Write( " {0}", e.Message );
		}

		public void On( DoneProcessingEvent e )
		{
			WithForeground( ConsoleColor.Green, () => Console.Write( " Done." ) );
		}
	}
}
