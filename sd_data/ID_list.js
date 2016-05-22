function checkPermission(perm)
{
	if (perm == "1") {
		return "<b><font color=\"green\">Dopuszczony</font></b>";
	} else if (perm == "0") {
		return "<b><font color=\"red\">Zablokowany</font></b>";
	} else if (perm == "2") {
		return "<b><font color=\"gray\">Nieznany</font></b>";
	}
}


function createTable(text)
{
	var table = "<table border=\"1\">";
	var temp;
	var cnt = 0;
	table += "<tr>";
	table += "<td>Numer ID</td>";
	table += "<td>Prawo dostępu</td>";
	table += "</tr>";
	
	temp = text.split(";");

	while(temp[cnt]) {
		table += "<tr>";
		table += "<td>" + temp[cnt] + "</td>";
		cnt++;
		table += "<td>" + checkPermission(temp[cnt]) + "</td>";
		cnt++;
		table += "</tr>";
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
		document.getElementById("table").innerHTML = createTable(allText);
            }
        }
    }
    rawFile.send(null);
}

readTextFile("ID_list.txt");