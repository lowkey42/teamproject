<?php
if (isset($_SERVER['HTTP_ORIGIN'])) {
    header("Access-Control-Allow-Origin: {$_SERVER['HTTP_ORIGIN']}");
    header('Access-Control-Allow-Credentials: true');    
    header("Access-Control-Allow-Methods: GET, POST, OPTIONS"); 
} else {
    header("Access-Control-Allow-Origin: *");
}

	// ------------------------------ //
	// DATABASE FORMAT                //
	// ------------------------------ //
	// ID: int(11) -> PRIMARY KEY     //
	// name: varchar(80)              //
	// level: varchar(40)             //
	// score: int(11)                 //
	// ------------------------------ //

	// final standard variables for database connection purpose
	$server = "TODO__SET_ME";
	$user = "TODO__SET_ME";
	$password = "TODO__SET_ME";
	$database = "TODO__SET_ME";
	$table = "itl_highscore";
		
	// try to read transmitted values (via GET, or POST)
	if($_POST || $_GET) {
		$level = ($_POST) ? $_POST["level"] : $_GET["level"];	
		$op = ($_POST) ? $_POST["op"] : $_GET["op"];
	} else {
		die("{\"ERROR\": \"NO GET OR POST\"}");
	}
	
	// Baue Datenbankverbindung auf
	$db = new mysqli($server, $user, $password, $database);	
	if($db->connect_errno){
		echo("Verbindungsaufbau zum Server fehlgeschlagen: ".$db->connect_errno);
		exit();
	}
	
	// ---------------
	// HIGHSCORE PUSH
	// ---------------
	
	if($op == "phigh") {
		
		// md5 check
		
		// read necessary values
		$name = ($_POST) ? $_POST["name"] : $_GET["name"];
		$time = ($_POST) ? $_POST["time"] : $_GET["time"];
		
		// check if the given score is a string and convert it
		if(is_string($time))
			$time = floatval($time);
	
		if($name != "" && $level != "" && $score >= 0) {
	
			// bereite Statement vor
			$statement = $db->prepare("INSERT INTO ".$table." (name, level, time) VALUES (?, ?, ?)");		
			if(!$statement){
				die("insert failed: ".$db->error);
			}
		
			// Prüfe Übergabe auf nicht unterstützte Zeichen
			$name = preg_replace('/[^a-zA-Z0-9_]/', '_', $name);
			$level = preg_replace('/[^a-zA-Z0-9_]/', '_', $level);
		
			// Binde Variablen im Statement
			$statement->bind_param("ssd", $name, $level, $time);
			if(!$statement->execute()){
				die("execution of insert statement failed: ".$statement->error);
			}
			
		} else {
			die("{\"ERROR\": \"UNSET VALUES\"}");
		}
		
	// --------------
	// HIGHSCORE GET
	// --------------
	
	} else if($op == "ghigh"){
				
		// bereite Statement vor, je nachdem ob ein Level angegeben wurde, oder die Levelangabe leer war 
		$statement = $db->prepare("SELECT `name`, `level`, `time` FROM ".$table." WHERE level = ? ORDER BY `time` ASC");
		
		if(!$statement){
			die("Error while preparing get-statement: ".$db->error);
		}
		
		// Binde Variablen im Statement, je nachdem ob ein Level angegeben wurde, oder die Levelangabe leer war
		$statement->bind_param("s", $level);
		
		if(!$statement->execute()){
			die("execution of select statement failed: ".$statement->error);
		}

		// Lese erhaltene Daten aus und speichere diese als Variablen
		$lev = $level;
		$name = $time = $level = NULL;
		$statement->bind_result($name, $level, $time);		
		
		// Gehe alle übergebenen Datenbankeinträge durch
		$first = true;
		printf("{\"level\": \"".$lev."\", \"scores\": [");
		while($statement->fetch()) {			
			if($first) {
				$first = false;
			} else {
				printf(",");
			}
			
			printf("\n  {\"name\": \"".$name."\", \"time\": ".$time."}");
		}
		
		printf("\n]}\n");
		
	} else {
		die("op not recognized -> FAILURE");
	}
	
	// Schließe Verbindung zur Datenbank
	if(!$db->close()){
		die("Error while closing db connection: ".$db->error);
	}
	
	/* Zeige Auslesedaten an:
	echo("<h2>Datenbankeinträge:</h2>");
	foreach($dictArr as $cur){
		echo("<p>NEUER EINTRAG!<br/>");
		foreach($cur as $key => $value){
			echo($key.": ".$value."<br/>");
		}
		echo("</p>");
	}
	
	// Zeige hinzuzufügende Informationen an
	echo("<br/><h2>Übermittelter Eintrag:</h2>");
	foreach($content as $key => $value){
		echo($key.": ".$value."<br/>");
	}
	echo("</p>");
	
		
		// Starte Anfrage an Datenbankverbindung und speichere Auslesedaten in ein Array
		// Auslesedaten werden dabei dicts sein, also ein Array von dicts
		$dictArr = array();
		while($cur = mysqli_fetch_assoc($result)){
			$dictArr[] = $cur;
		}*/
	
?>
