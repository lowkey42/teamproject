<?php

	// ------------------------------ //
	// DATABASE FORMAT                //
	// ------------------------------ //
	// ID: int(11) -> PRIMARY KEY     //
	// game: varchar(40)              //
	// name: varchar(80)              //
	// level: varchar(40)             //
	// score: int(11)                 //
	// ------------------------------ //

	// final standard variables for database connection purpose
	$server = "localhost";
	$user = "root";
	$password = "";
	$database = "test";
	$table = "tphighscore";
		
	// try to read transmitted values (via GET, or POST)
	if($_POST || $_GET){
		//$game = ($_POST) ? $_POST["game"] : $_GET["game"];		
		$game = "Into_the_light";
		$level = ($_POST) ? $_POST["level"] : $_GET["level"];	
		$op = ($_POST) ? $_POST["op"] : $_GET["op"];
	} else {
		printf("{\"ERROR\": \"NO GET OR POST\"}");
		exit();
	}
	
	// Baue Datenbankverbindung auf
	$db = new mysqli($server, $user, $password, "test");	
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
		$level = ($_POST) ? $_POST["level"] : $_GET["level"];
		$score = ($_POST) ? $_POST["score"] : $_GET["score"];
		
		// check if the given score is a string and convert it
		if(is_string($score))
			$score = intval($score);
	
		if($name != "" && $level != "" && $score >= 0) {
	
			// bereite Statement vor
			$statement = $db->prepare("INSERT INTO ".$table." (game, name, level, score) VALUES (?, ?, ?, ?)");		
			if(!$statement){
				die("insert failed: ".$db->error);
			}
		
			// Prüfe Übergabe auf nicht unterstützte Zeichen
			$game = preg_replace('/[^a-zA-Z0-9_]/', '_', $game);
			$name = preg_replace('/[^a-zA-Z0-9_]/', '_', $name);
	
			// Binde Variablen im Statement
			$statement->bind_param("sssi", $game, $name, $level, $score);
			if(!$statement->execute()){
				die("execution of insert statement failed: ".$statement->error);
			}
			
		} else {
			
			printf("{\"ERROR\": \"UNSET VALUES\"}");
			exit();
			
		}
		
	// --------------
	// HIGHSCORE GET
	// --------------
	
	} else if($op == "ghigh"){
				
		// bereite Statement vor, je nachdem ob ein Level angegeben wurde, oder die Levelangabe leer war 
		$statement = $db->prepare("SELECT `name`, `level`, `score` FROM ".$table." WHERE game = 'Into_the_light' and level = ? ORDER BY `score` DESC");
		
		if(!$statement){
			die("Error while preparing get-statement: ".$db->error);
		}
		
		// Prüfe Übergabe auf nicht unterstützte Zeichen
		$game = preg_replace('/[^a-zA-Z0-9_]/', '_', $game);
		
		// Binde Variablen im Statement, je nachdem ob ein Level angegeben wurde, oder die Levelangabe leer war
		$statement->bind_param("s", $level);
		
		if(!$statement->execute()){
			die("execution of select statement failed: ".$statement->error);
		}

		// Lese erhaltene Daten aus und speichere diese als Variablen
		$lev = $level;
		$name = $score = $level = NULL;
		$statement->bind_result($name, $level, $score);		
		
		// Gehe alle übergebenen Datenbankeinträge durch
		$entries = array();
		printf("{\n");
		while($statement->fetch()){			
			// printf("\t{\"name\": \"%s\", \"score\": %s},\n", $name, $score);	
			$entries[] = ("\t[\"name\": \"".$name."\", "
							.(($lev != "") ? ("") : ("\"level\": ".$level.", "))
							."\"score\": ".$score."],\n");	
		}
		
		// Entferne letztes Komma, welches nicht benötigt wird
		if(count($entries) > 0){
			$lastIndex = count($entries) - 1;
			$lastEntry = $entries[$lastIndex];
			$entries[$lastIndex] = substr($lastEntry, 0, count($lastEntry) - 3)."\n";
		}

		// Schreibe Einträge JSON-konform aus
		foreach($entries as $entry){
			printf($entry);
		}
		
		printf("}");
		
	} else {
		print_r("op not recognized -> FAILURE");
		exit();
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