using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <string.h>
#include<stdlib.h>

struct cmpFnStruct{
    const char * actorName;
    const void * actorFile;
  };

struct cmpFn2Struct{
  film movie;
  const void * movieFile;
};

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

  int cmpFn(const void * vp1, const void * vp2);
  int cmpFn2(const void * vp1, const void * vp2);


imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

/* Comparator function for bsearch for getCredits
 * vp1 is the pointer to key (player)
 * and vp2 is the pointer to the offset of the record.
 * 
 */
int cmpFn(const void * vp1, const void * vp2) {
  cmpFnStruct * elem1 = (cmpFnStruct *) vp1;  //typecasting key as char *
  int * elem2 = (int *) vp2; //vp2 is the pointer to the offset which is an int

  const char * actorToBeFound = elem1->actorName;
  const void * actorFile = elem1->actorFile;

  //Comparing the names pointed to by the key and (baseAdd + offset) 
  return strcmp(actorToBeFound, ((char *)actorFile + *elem2));
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const
{ 
  //Variables that are being passed to the bsearch
  cmpFnStruct actorToBeFound;
  actorToBeFound.actorName = player.c_str();
  actorToBeFound.actorFile = actorFile;
 
  int numActors = *((int*)actorFile);
  int elemSize = sizeof(int);

  //Searching for the player in the actorFile.
  //The array storing the offsets is passed to the bsearch. i.e (actorFile + sizeof(int)).
  //Returns the value of the offset. bsearch returns a void pointer, hence typecasting it to int* 
  //and dereferencing.
  void * playerOffset = 
    bsearch(&actorToBeFound, (char*)actorFile + sizeof(int), numActors, elemSize, &cmpFn);
  
  if(playerOffset == NULL){
    return false;
  }

  else{
    void * playerAddr = (char *)actorFile + *(int *)playerOffset;
    
    // Get the name of the player
    string playerName = getActorName(playerAddr);
    // Get the complete list of movies that this player acted in
    getMovieOffsets(playerAddr, films, playerName);

    return true;
  }
}

//The following function returns the name of the actor.
//A pointer pointing to an actor's record is passed as the input values
string imdb::getActorName(void * playerAddr) const {
  
  string playerName = "";
  
  while(*(char*)playerAddr != '\0'){
    playerName = playerName + *(char*)playerAddr;
    playerAddr = (char *)playerAddr + 1;
  }
  return playerName;
}


//Implement getMovieOffsets function. Takes in the pointer pointing to the beginning of the player record 
//and the vector<film> that needs to be populated with the name of the movies.
//Actor name is also passed as a parameter to help in the finding the length.
void imdb::getMovieOffsets(void * playerAddr, vector<film> &films, string playerName) const {

  //To get to the movie offsets, we need to advance the pointer ahead of the actor name and the number of 
  //movies the actor acted in.
  int i = 0;
  int playerLength = playerName.length();
  
  //If the length of the actor name is even, two '\0's will be padded in the record. Else, one '\0'.
  if(playerLength % 2 == 0)
    i = playerLength + 2;
  else
    i = playerLength + 1;

  //ptr points to the address at the end of the actorName and the padded '\0's
  playerAddr = (char *)playerAddr + i;

  short numOfMovies = *(short *)playerAddr;
  playerAddr = (short*)playerAddr + 1;
  i += 2;
  
  //If the number of bytes used to store the actor name and the number of movies is not divisible by 4, then two bytes are stored with '\0'
  if(i % 4 != 0)
    playerAddr = (char *)playerAddr + 2;
  
  //Creating an array holding the movie offsets for the given actor.
  int movieOffset[numOfMovies];
  for(int j = 0; j < numOfMovies; j++){
    movieOffset[j] = *((int *)playerAddr);
    playerAddr = (int *)playerAddr + 1;
  }

  getMovies(films, movieOffset, numOfMovies);
}

/*  Getting the name of the movie and adding to the vector<film>
 *  The array containing the offsets of the movie is passed in as a parameter.
 *  The name of the movie is extracted from the movie record and added to the vector.
 */
void imdb::getMovies(vector<film> &films, int movieOffset[], int numOfMovies) const {

  void * movieRecord = NULL;

  film movie;
  string movieName = "";

  for(int i=0; i < numOfMovies; i++){
    movieRecord = (char *)movieFile + movieOffset[i];
    movieName = "";
    while(*(char *)movieRecord != '\0'){
      movieName += *((char *)movieRecord);
      movieRecord = (char *)movieRecord + 1;
    }
    movie.title = movieName;
    movieRecord = (char *)movieRecord + 1;
    movie.year = 1900 + (int)(*((char*)movieRecord));
    films.push_back(movie);
  }
}

/*   Comparator function for bsearch used in getCast().
 *   Comparing the movie title and the year.
 */
int cmpFn2(const void * vp1, const void * vp2) {

  cmpFn2Struct * elem1 = (cmpFn2Struct *)vp1; //Typecasting key
  int * elem2 = (int *)vp2; //Returns to the pointer to the offset of the movie
  
  const film filmKey = elem1->movie;
  const void * movieFile = elem1->movieFile;
  
  void * filmInfo = ((char *)movieFile + *elem2); //Getting the film record from the given offset.

  //If the title of the movies match, we check for the year.
  if (strcmp(filmKey.title.c_str(), (char *)filmInfo) == 0){
    void * ptr = (char *)filmInfo + filmKey.title.length() + 1; //1 for the '\0'
    int year = (int)(*(char*)ptr + 1900);
    if(year == filmKey.year){
      return 0;
    }
    else if(year < filmKey.year)
      return 1;
    else 
      return -1;
  }
  else {
    return strcmp(filmKey.title.c_str(), (char *)filmInfo);
    }
 
}

/*  Given a movie, this function returns the cast(all the actors) of the movie.
 */
bool imdb::getCast(const film& movie, vector<string>& players) const { 

  int numOfMovies = *((int*)movieFile);

  cmpFn2Struct movieToBeFound;
  movieToBeFound.movie.title = movie.title;
  movieToBeFound.movie.year = movie.year;
  movieToBeFound.movieFile = movieFile;

  //Search for the movie using bsearch and find the offset. If offset is NULL, return false.
  void * movieOffset = bsearch(&movieToBeFound, (char *)movieFile + sizeof(int), numOfMovies, sizeof(int),
			       &cmpFn2);

  if(movieOffset == NULL){
    return false;
  }

  else{
    void * movieRecord = (char *)movieFile + *(int *)movieOffset;

    //Get the film record(title and year)
    film movie = getMovie(movieRecord);
    //Get the cast offsets and populate the vector
    getMovieCast(movieRecord, movie, players);

    return true;
  } 
}

/*  When a pointer pointing to the movieRecord is passed, this function returns the film.
 *  film contains the movie title and year of release.
 */
film imdb::getMovie(void * movieRecord) const {
  film movie;

  while(*(char*)movieRecord != '\0'){
    movie.title += *(char *)movieRecord;
    movieRecord = (char *)movieRecord + 1;
  }
   movieRecord = (char *)movieRecord + 1;
   movie.year = 1900 + (int)(*(char *)movieRecord);

   return movie;
}

/*  This functions gives the cast of a movie
 *  A pointer pointing to the movie record is passed in as a parameter.
 */
void imdb::getMovieCast(void * movieRecord, film movie, vector<string> &players) const {
  
  int i = 0;
  //filLength gives the length of the movie name + one byte for '\0' + one byte for the year.
  int filmLength = (movie.title).length() + 2;

  movieRecord = (char *)movieRecord + filmLength;
  i = filmLength;

  if(i % 2 != 0){
    movieRecord = (char *)movieRecord + 1;
    i++;
  }
  
  //Number of actors in the given movie.
  short numOfActors = *(short *)movieRecord;
  movieRecord = (short *)movieRecord + 1;
  i = i + 2;

  //cout << "FilmLength: " << filmLength << "\ti: " << i << endl;

  if(i % 4 != 0){
    movieRecord = (char *)movieRecord + 2;
  }

  int actorOffsets[numOfActors];
  //cout << "Actor Offsets: " << endl;
  for(int j = 0; j < numOfActors; j++){
    actorOffsets[j] = *((int *)movieRecord);
    //cout << actorOffsets[j] << "\t";
    movieRecord = (int *)movieRecord + 1;
  }
  //cout << endl << endl;
   //Populate the vector of players with the names of the actors.
  getActors(players, actorOffsets, numOfActors);

}

/*   This function is used to populate the vector with the actors of a given film.
 *   The array containing the offsets of an actorFile is passed. 
 */
void imdb::getActors(vector<string> &players, int actorOffsets[], int numOfActors) const {
  
  string actorName = "";
  void * actorRecord = NULL;

  for(int i = 0; i < numOfActors; i++){
    actorRecord = (char *)actorFile + actorOffsets[i];
    actorName = "";
    while(*(char *)actorRecord != '\0'){
      actorName += *(char *)actorRecord;
      actorRecord = (char *)actorRecord + 1;
    }
    players.push_back(actorName);
  }
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
