namespace AutoTest
{
    public class Map
    {
        private string path;
        private Platforms platform;

        public Map( string path, Platforms platform )
        {
            this.path = path;
            this.platform = platform;
        }

        public string Path
        {
            get { return path; }
        }

        public Platforms Platform
        {
            get { return platform; }
        }
    }
}
