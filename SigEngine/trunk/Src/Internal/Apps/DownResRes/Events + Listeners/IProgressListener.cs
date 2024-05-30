
namespace DownResRes
{
	interface IProgressListener
	{
		void On( StartProcessingEvent e );
		void On( ErrorProcessingEvent e );
		void On( InfoProcessingEvent e );
		void On( DoneProcessingEvent e );
	}
}
