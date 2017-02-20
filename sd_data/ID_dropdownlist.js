function createDropDownList(text)
{
	var element = '';
	var temp;
	var cnt = 0;
	
	temp = text.split(";");
		
	while(temp[cnt]) {
		element += "<option value=\"";
		element +=  cnt/2;
		element += "\">";
		element += temp[cnt];
		element += "</option>";
		cnt += 2;
	}

	return element;
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
		document.getElementById("L0").innerHTML = createDropDownList(allText);
            }
        }
    }
    rawFile.send(null);
}

readTextFile("ID_list.txt");