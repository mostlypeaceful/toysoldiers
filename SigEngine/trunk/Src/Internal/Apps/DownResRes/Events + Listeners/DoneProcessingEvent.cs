
namespace DownResRes
{
	struct DoneProcessingEvent
	{
		public string File;

		public DoneProcessingEvent( string file )
		{
			File = file;
		}
	}
}
