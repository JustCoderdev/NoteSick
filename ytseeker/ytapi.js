const fs = require('node:fs');


const APIKEY_PATH = "youtube-api.key";
const PLAYLIST_ID = "PLd23Y4uu3SslprRLNuBitQft8kb4a7R3q";


/* Initialisation */
/* ------------------------------------------------------------ */

const apikey_res = slurpFile(APIKEY_PATH);
if(apikey_res.error != null)
{
	const e = apikey_res.error;
	console.error(`Could not open key file '${e.path}': ${e.code}`)
	process.exit(1);
}
const apikey = apikey_res.data;
console.log("Successfully slurped api key");

// const url = new URL("https://www.googleapis.com/youtube/v3/playlists");
const url = new URL("https://www.googleapis.com/youtube/v3/playlistItems");
url.searchParams.append("key", apikey);
url.searchParams.append("part", "contentDetails");
url.searchParams.append("playlistId", PLAYLIST_ID);

/* main */
/* ------------------------------------------------------------ */

getJson(url)
	.then(playlist => {

		console.log(`Found ${playlist.pageInfo.totalResults} videos in this playlist`)

		playlist.items.forEach(video => {
			console.log(video.contentDetails)
		})

		// const playlist = playlists.items[0];
		// console.log(`Playlist ${playlist.id} has ${playlist.contentDetails.itemCount} videos`);
	})


/* Helpers */
/* ------------------------------------------------------------ */

function slurpFile(path)
{
	try {
		const data = fs.readFileSync(path, 'utf8');
		return { data: data, error: null };
	} catch (error) {
		return { data: null, error: error };
	}
}

async function getJson(endpoint)
{
	const res = await fetch(endpoint, {
		method: "GET",
		headers: {
			"Content-Type": "application/json",
		},
		// body: JSON.stringify({  })
	});

	const data = await res.json();
	// console.log("------------------------------");
	// console.log(data);
	// console.log("------------------------------");

	if(!res.ok)
	{
		console.error("\x1B[31mReturned not OK response\x1B[0m");
	}

	return data;
}
