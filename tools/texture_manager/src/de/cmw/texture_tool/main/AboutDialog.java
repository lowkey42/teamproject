package de.cmw.texture_tool.main;

import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class AboutDialog {

	public AboutDialog() {}
	
	public static void showDialog(){
		
		Stage modalStage = new Stage();
		modalStage.initModality(Modality.APPLICATION_MODAL);
		
		modalStage.setScene(generateScene());
		modalStage.setTitle("About");
		modalStage.setResizable(false);
		modalStage.show();
		
	}
	
	private static Scene generateScene(){
		
		HBox root = new HBox();		
		Scene scene = new Scene(root);
		
		Label infoTxt = new Label();
		//infoTxt.setEditable(false);
		infoTxt.setText(readAboutTxt());
		infoTxt.setStyle("-fx-text-fill: #000000; -fx-background-color: #F3F3F3");
		
		// Adding childs
		root.getChildren().add(infoTxt);
		HBox.setMargin(infoTxt, new Insets(10, 10, 10, 15));
		
		return scene;		
		
	}
	
	private static String readAboutTxt(){
		
		//String ret = "";
		
		String hardCodedAboutInfo = "﻿v0.4 (c) Copyright by Sebastian Schalow\n"
								  + "Tool entwickelt für das Teamprojekt\n\n"
								  + "\"Into The Light\" im SS2016\n"
								  + "an der Hochschule Trier\n\n"
								  +	"Vervielfältigung und kommerzielle Nutzung\n"
								  + "ohne Einwilligung des Entwicklers stellen\n"
								  + "eine strafrechtliche Handlung dar. Das\n"
								  + "geistige Eigentum verbleibt beim Entwickler.\n";
		
		// TODO --> Read from file without having a concrete class (static class)
		/*File fileToRead = new File(AboutDialog.class.getName());
		
		if(fileToRead.exists()){
			try {
				FileReader fr = new FileReader(fileToRead);
				BufferedReader br = new BufferedReader(fr);
				StringBuffer strBuf = new StringBuffer();
				while(br.ready()){
					strBuf.append(br.readLine() + "\n");
				}
				br.close();
				ret = strBuf.toString();
			} catch (IOException e) {
				e.printStackTrace();
			}
		} else {
			ret = "About-Info not found!";
			System.err.println("File " + fileToRead.getName() + " does not exist!");
			System.err.println("current path: " + System.getProperty("user.dir"));
		} 
		
		return ret;*/
		
		return hardCodedAboutInfo;
		
	}
	
}
