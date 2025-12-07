#ifndef DJLIBRARYSERVICE_H
#define DJLIBRARYSERVICE_H

#include "Playlist.h"
#include "AudioTrack.h"
#include "SessionFileParser.h"
#include <vector>
#include <string>

// Service responsible for managing the track library and playlists
// Phase 4 behavior alignment:
// - Load library tracks from config file
// - Build playlists from track indices referencing the library
class DJLibraryService {
public:
    /*
     * Constructors
     */
    DJLibraryService(const Playlist& playlist);
    DJLibraryService() = default;

    /*
     * Destructor
     */
    ~DJLibraryService();

    /*
     * Copy Constructor
     */
    DJLibraryService(const DJLibraryService&);

    /*
     * Copy Assignment Operator
     */
    DJLibraryService& operator=(const DJLibraryService&);


    /**
     * @brief Build the track library from parsed config data
     * @param library_tracks Vector of track info from config
     */
    void buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks);

    /**
     * @brief Load a playlist by constructing it from track indices
     * @param playlist_name Name of the playlist
     * @param track_indices Vector of 1-based track indices referencing the library
     */
    void loadPlaylistFromIndices(const std::string& playlist_name, const std::vector<int>& track_indices);

    // Returns a reference to the loaded playlist
    Playlist& getPlaylist();

    // Display all playlists in the library (debug aid; optional for Phase 4)
    void displayLibrary() const;

    /**
     * @brief Find a track in the library by its title.
     * @param track_title The title of the track to find.
     * @return A raw pointer to the AudioTrack if found, otherwise nullptr.
     * The library retains ownership of the track.
     * החלטתי להפוך את המתודה לקונסט כי המצביע לא אמור להשתנות מחוץ למחלקה,
     *  וכי נכון יותר שזה יהיה הטיפוס המוחזר.
     */
    const AudioTrack* findTrack(const std::string& track_title) const;

    /**
     * @brief Get a vector of all track titles in the current playlist.
     * @return A vector of strings containing the track titles.
     */
    std::vector<std::string> getTrackTitles() const;

    //helper function: swap
    void swap(DJLibraryService&);

private:
    Playlist playlist;
    std::vector<AudioTrack*> library;  // Library of all tracks (owned)
};

#endif // DJLIBRARYSERVICE_H
