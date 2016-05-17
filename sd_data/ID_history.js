function createTable(text)
{
	var table = "<table border=\"1\">";
	var record;
	var element;
	var i = 0;
	var j = 0;

	table += "<tr>";
	table += "<td>Data</td>";
	table += "<td>Godzina</td>";
	table += "<td>Numer ID</td>";
	table += "</tr>";
	
	record = text.split(";");

	while(record[i]) {
		element = record[i].split(",");
		table += "<tr>";

		j = 0;
		while(element[j]) {
			table += "<td>" + element[j] + "</td>";
			j++;
		}

		table += "</tr>";
		i++;
	}
	table += "</table>";
	return table;
}


function readTextFile(file)
{
    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", file, false);
    rawFile.onreadystatechange = function ()
    {
        if(rawFile.readyState === 4)
        {
            if(rawFile.status === 200 || rawFile.status == 0)
            {
                var allText = rawFile.responseText;
		document.getElementById("history_table").innerHTML = createTable(allText);
            }
        }
    }
    rawFile.send(null);
}

readTextFile("history.txt");