using System;
using System.Collections.Generic;
using System.Net;
using Newtonsoft.Json.Linq;

namespace Prefinery_Inviter
{
    class Program
    {
        const string apiKey = "api_key=<api_key_goes_here>";
        const string betaID = "3538";
        const string baseURL = "https://signalstudios.prefinery.com/api/v2/";

        static WebClient webClient = new WebClient( );

        static int invitationLimit = 0;

        static void Main( string[ ] args )
        {
            int menuSelection = GetMenuSelection( );
            switch( menuSelection )
            {
                case 1:
                    {
                        string email = GetEmailAddress( );

                        JArray testers = GetTesterByEmail( email );
                        if( testers.Count < 1 )
                            Console.WriteLine( "No tester found with that address!" );
                        else
                        {
                            foreach( JObject tester in testers )
                                InviteTester( tester );
                        }
                    }
                    break;
                case 2:
                    {
                        invitationLimit = GetInvitationLimit( );

                        List<JObject> testers = GetTestersAwaitingInvite( );

                        int invited = 0;
                        while( invited < invitationLimit && invited < testers.Count )
                            InviteTester( testers[ invited++ ] );
                        if( invited >= invitationLimit )
                            Console.WriteLine( "The invitation limit has been reached." );
                        else
                            Console.WriteLine( "All awaiting testers have been invited." );
                    }
                    break;
                default:
                    Console.Write( "Invalid menu selection." );
                    break;
            }

            Console.WriteLine( "Press any key to continue..." );
            Console.ReadKey( true );
        }

        static int GetMenuSelection( )
        {
            int menuSelection = 0;
            do
            {
                Console.WriteLine( "Please choose one of the following options:\n1) Invite user by email address.\n2) Mass invite users." );
                Console.Write( "Enter choice: " );
                string response = Console.ReadLine( );
                try
                {
                    menuSelection = Convert.ToInt32( response );
                }
                catch
                {
                    Console.WriteLine( "Please enter a value between 1-2!");
                    menuSelection = 0;
                }
            }
            while( menuSelection < 1 || menuSelection > 2 );

            return menuSelection;
        }

        static string GetEmailAddress( )
        {
            string email = String.Empty;
            do
            {
                Console.Write( "Enter the email address to look up: " );
                email = Console.ReadLine( );
                if( email == String.Empty || !email.Contains( "@" ) )
                    Console.WriteLine( "Please enter a valid email address!" );
            }
            while( email == String.Empty || !email.Contains( "@" ) );

            return email;
        }

        static int GetInvitationLimit( )
        {
            int invitationLimit = 0;
            do
            {
                Console.Write( "Enter the maximum number of invitations to send: " );
                string response = Console.ReadLine( );
                try
                {
                    invitationLimit = Convert.ToInt32( response );
                }
                catch
                {
                    Console.WriteLine( "Please enter a numeric value!" );
                    invitationLimit = 0;
                }
            }
            while( invitationLimit < 1 );

            return invitationLimit;
        }

        static List<JObject> GetTestersAwaitingInvite( )
        {
            // Get a list of testers awaiting an invitation
            List<JObject> testers = new List<JObject>( );
            for( int i = 1; true; ++i )
            {
                // Get the a page of testers
                Console.WriteLine( "Getting testers..." );
                string response = webClient.DownloadString( baseURL + "/betas/" + betaID + "/testers.json?" + apiKey + "&page=" + i );

                // Add the testers to our list
                Console.WriteLine( "Parsing testers..." );
                JArray tokens = JArray.Parse( response );
                for( int j = 0; j < tokens.Count; ++j )
                {
                    JToken token = tokens[ j ];
                    JObject tester = token.ToObject<JObject>( );

                    // We're looking for testers who have a status of "applied" and have an assigned game token;
                    // those without a game token aren't ready and those who have any other status are not ready
                    string status = tester[ "status" ].ToString( );
                    string email = tester[ "email" ].ToString( );
                    string gameToken = JObject.Parse( tester[ "profile" ].ToString( ) )[ "employer" ].ToString( );
                    if( status == "applied" && gameToken != String.Empty )
                    {
                        Console.WriteLine( "Added " + email + "..." );
                        testers.Add( tester );
                        if( testers.Count >= invitationLimit )
                            break;
                    }
                }
                if( testers.Count >= invitationLimit )
                    break;

                // Exit if there's no pages left
                WebHeaderCollection responseHeaders = webClient.ResponseHeaders;
                JObject paginationHeader = JObject.Parse( responseHeaders[ "X-Pagination" ] );
                Console.WriteLine( "Completed page " + i + "/" + paginationHeader[ "pages" ].ToString( ) + "." );
                if( paginationHeader[ "next_page" ].ToString( ) == String.Empty )
                {
                    Console.WriteLine( "There are no more pages." );
                    break;
                }
            }

            return testers;
        }

        static void InviteTester( JObject tester )
        {
            JToken email = tester[ "email" ];
            if( email == null )
                return;

            JToken id = tester[ "id" ];
            if( id == null )
                return;

            Console.WriteLine( "Inviting " + email.ToString( ) + "...");
            string url = baseURL + "betas/" + betaID + "/testers/" + id + ".json?" + apiKey;
            string request = "{\"tester\":{\"status\":\"invited\",\"profile\":{}}}";
            webClient.Headers.Add( "Accept", "application/json" );
            webClient.Headers.Add( "Content-type", "application/json" );
            string response = webClient.UploadString( url, "PUT", request );
        }

        static JArray GetTesterByEmail( string email )
        {
            string url = baseURL + "betas/" + betaID + "/testers.json?" + apiKey + "&email=" + email;
            string response = webClient.DownloadString( url );
            return JArray.Parse( response );
        }
    }
}
