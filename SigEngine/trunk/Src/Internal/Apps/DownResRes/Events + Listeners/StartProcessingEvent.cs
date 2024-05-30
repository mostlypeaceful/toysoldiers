
namespace DownResRes
{
	struct StartProcessingEvent
	{
		public string File;
		public int FileNumber;
		public int FilesCount;

		public StartProcessingEvent( string file, int filenum, int filecount )
		{
			File = file;
			FileNumber = filenum;
			FilesCount = filecount;
		}
	}
}
