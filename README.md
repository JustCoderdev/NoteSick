# NoteSick

I like having my music library always with me and I repeat the same steps every
time I need to download new music, I want to automate this

## The process

> Note: this is C code without error handling

```c
Song newsong = discover_new_song();
yt_playlist_append(&music_pl, newsong.url);

yt_playlist_curate(&music_pl);

Songs songs = yt_playlist_download(music_pl);
for(int i = 0; i < songs.count; ++i)
{
    Song song = songs.items[i];

    Metadata meta = song_fetch_meta(song.title);
    song_update_meta(&song, meta);
}
````

## The plan

Having written down the process now I know how to break down the problem

The project will be divided in 5 subprojects, each with the specific goal of
constructing a library that I can use in the final application

- ytseeker: access to YT api --> fetch playlist/video information
- ytdownloader: download YT stream from url

- metareader: dump of song file metadata
- metaseeker: find on the internet metadata from song name
- metawriter: write to song file provided metadata

## Goals

- Create an OpenGL library for UI
- Support metadata from WAV, FLAC and MP3 files
- Download videos from youtube
- Create a decent json parser
