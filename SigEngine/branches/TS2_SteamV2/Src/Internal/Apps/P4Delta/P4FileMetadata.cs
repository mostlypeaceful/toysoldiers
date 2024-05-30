namespace P4Delta
{
    class P4FileMetadata
    {
        public string depotFile = "";   // The file's location relative to the top of the depot
        public string rev = "";         // Revision number of the head revision of that file
        public string change = "";      // The number of the changelist that this revision was submitted in
        public string action = "";      // The action taken at the head revision: add, edit, delete, branch, or integrate
        public string type = "";        // The Perforce file type of this file at the head revision.
        public string time = "";
    }
}
