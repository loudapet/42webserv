function backgroundRandomiser() {
	var r = Math.floor(Math.random() * 256);
	var g = Math.floor(Math.random() * 256);
	var b = Math.floor(Math.random() * 256);
	var string = "rgb(" + r.toString() + ", " + g.toString() + ", " + b.toString() + ")";
	document.body.style.background = string;
  }