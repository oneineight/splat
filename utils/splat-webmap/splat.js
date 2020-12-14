/* SPLAT! web viewer "splat.js"
 * A simple tool for loading a georeferenced PNG image into a Leaflet based web map.
 * 
 * Viewing radio propagation maps is very easy:
 * - Let SPLAT! generate JSON along with PNG output
 * - Open the JSON file and parse it with splat_render. You have two options:
 *   - Load a local file from disk with HTML5 interaction (function "splat_file")
 *   - Load a file from a webserver via AJAX (function "splat_ajax")
 * 
 * (c) 2020 Stefan Erhardt and contributors
 *  
 */

function splat_render(splat) {
	/* parses JSON file from SPLAT! and displays it */
	map.setView(splat.qth.coordinates, 11);
	L.imageOverlay(splat.image.file, splat.image.bounds, {
			opacity: 0.7
	}).addTo(map);
	
	L.marker(splat.qth.coordinates).addTo(map)
			.bindPopup("<h3>"+splat.name+"</h3>height: "+splat.qth.height+" m AGL<br/>power: "+splat.lrp.erp+" W");
			
	var info = document.getElementById("info");
	info.innerHTML = "<pre>"+JSON.stringify(splat,null,4)+"</pre>";
}

function splat_ajax(file) {
	/* AJAX file loader for json files from web server */
	var response, request = new XMLHttpRequest();
	request.open("GET", file);
	request.onreadystatechange = function() {
		if (request.readyState === 4 && request.status === 200) {
			response = JSON.parse(request.responseText);
			splat_render(response);
		}
	};
	request.send();
}

function splat_file(event) {
	/* Local file loader from file system */
	var file = event.target;
	var reader = new FileReader();
	reader.onload = function() {
		var response = reader.result;
		splat_render(JSON.parse(response))
	};
	reader.readAsText(file.files[0]);
}
