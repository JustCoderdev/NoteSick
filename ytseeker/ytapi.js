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

const GET_PLAYLIST_ITEMS_COUNT_URL = new URL("/youtube/v3/playlists", "https://www.googleapis.com");
const params = new searchParams();
params.append("key", apikey);
params.append("part", "contentDetails"); // ,snippet
params.append("id", PLAYLIST_ID);
GET_PLAYLIST_ITEMS_COUNT_URL.searchParams = params

// TODO: FINISH GET_PLAYLIST_ITEMS_URL
// const GET_PLAYLIST_ITEMS_URL = new URL("/youtube/v3/playlists", "https://www.googleapis.com");
// const params = new searchParams();
// params.append("key", apikey);
// params.append("part", "contentDetails"); // ,snippet
// params.append("id", PLAYLIST_ID);
// GET_PLAYLIST_ITEMS_COUNT_URL.searchParams = params


/* main */
/* ------------------------------------------------------------ */

getJson(GET_PLAYLIST_ITEMS_COUNT_URL)
	.then(playlists => {

		if(playlists.pageInfo.totalResults != 1)
		{
			console.error(`Found ${playlists.pageInfo.totalResults} playlists with the same id!`)
			process.exit(1);
		}

		const playlist = playlists.items[0];
		console.log(`Playlist ${playlist.id} has ${playlist.contentDetails.itemCount} videos`);
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
	console.log("------------------------------");
	console.log(data);
	console.log("------------------------------");

	return data;
}
