using System;

namespace DownResRes
{
	struct ErrorProcessingEvent
	{
		public string File;
		public string Description;
		public Exception Exception;

		public ErrorProcessingEvent( string file, string description )
		{
			File = file;
			Description = description;
			Exception = null;
		}

		public ErrorProcessingEvent( string file, string description, Exception e )
		{
			File = file;
			Description = description;
			Exception = e;
		}
	}
}
