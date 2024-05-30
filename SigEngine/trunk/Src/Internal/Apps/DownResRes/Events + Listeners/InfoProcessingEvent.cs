
namespace DownResRes
{
	struct InfoProcessingEvent
	{
		public string File;
		public string Message;

		public InfoProcessingEvent( string file, string message )
		{
			File = file;
			Message = message;
		}
	}
}
