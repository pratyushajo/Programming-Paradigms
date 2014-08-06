#include <vector>
#include <list>
#include <set>
#include <map>
#include <string.h>
#include <iostream>
#include <iomanip>
#include "imdb.h"
#include "path.h"
using namespace std;

void generateShortestPath(string source, string target, imdb &db);
bool movieSeen(film &movie, set<film> &previouslySeenFilms);
bool castSeen(string &castMember, set<string> &previouslySeenActor);
void printPath(path &newPath);
bool creditsSeen(string &lastActor, map<string, vector<film> > &previouslySeenCredits);
bool castSeen(film &movie, map<film, vector<string> > &castMap);

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}


/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

int main(int argc, const char *argv[])
{
  imdb db(determinePathToData("/Users/pratyushaj/Documents/Programming Paradigms/assignments/assn-2-six-degrees-data/little-endian")); // inlined in imdb-utils.h
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    exit(1);
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      // replace the following line by a call to your generateShortestPath routine... 
      generateShortestPath(source, target, db);
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}

void generateShortestPath(string source, string target, imdb &db){
  list<path> partialPath;
  set<string> previouslySeenActors;
  set<film> previouslySeenFilms;
  vector<film> moviesActedByActor;
  vector<string> movieCast;

  //Memoizing the credits and the cast 
  map<string, vector<film> > creditMap;
  map<film, vector<string> > castMap;

  path initialPath(source);
  partialPath.push_front(initialPath);
  
  while(!partialPath.empty() && partialPath.front().getLength() <= 5){

    path front = partialPath.front();
    partialPath.pop_front();
    string lastActor = front.getLastPlayer();

    if(!creditsSeen(lastActor, creditMap)){
        db.getCredits(lastActor, moviesActedByActor);
	creditMap[lastActor] = moviesActedByActor;
      }
      else
	moviesActedByActor = creditMap[lastActor];
    
    //db.getCredits(lastActor, moviesActedByActor);

       for(int i = 0; i < moviesActedByActor.size(); i++){
	 film movie = moviesActedByActor[i];
	 if(!movieSeen(movie, previouslySeenFilms)){
	   previouslySeenFilms.insert(movie);
	   
	   if(!castSeen(movie, castMap)){
	     db.getCast(movie, movieCast);
	     castMap[movie] = movieCast;
	   }
	   else
	     movieCast = castMap[movie];

	   //db.getCast(movie, movieCast);
	   for (int j = 0; j < movieCast.size(); j++){
	     string castMember = movieCast[j];
	     if(!castSeen(castMember, previouslySeenActors)){
	       previouslySeenActors.insert(castMember);
	       path newPath = front;
	       newPath.addConnection(movie, castMember);
	       if(castMember == target){
	         printPath(newPath);
	         return;
	       }
	       else{
	         partialPath.push_back(newPath);
	       }
	     }
	   }
	 }
       }
  }
  
  cout << endl << "No path between those two people could be found." << endl << endl;
}

bool movieSeen(film &movie, set<film> &previouslySeenFilms){
  return (previouslySeenFilms.find(movie) != previouslySeenFilms.end());
}

bool castSeen(string &castMember, set<string> &previouslySeenActor){
  return (previouslySeenActor.find(castMember) != previouslySeenActor.end());
}

bool creditsSeen(string &lastActor, map<string, vector<film> > &previouslySeenCredits){
  return (previouslySeenCredits.find(lastActor) != previouslySeenCredits.end());
}

bool castSeen(film &movie, map<film, vector<string> > &castMap){
  return (castMap.find(movie) != castMap.end());
}

void printPath(path &newPath){
  cout << newPath << endl;
}
