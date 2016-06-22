package de.cmw.texture_tool.utils;

import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

public class InfoDialog {
	
	private static Stage stage;
	
	/**
	 * Shows an Dialog with an OK-Button and the given parameter <i>infoTxt</i>.
	 * @param infoTxt
	 */
	public static void showErrorDialog(String infoTxt){
		
		stage = new Stage();
		stage.initModality(Modality.APPLICATION_MODAL);		
		stage.initStyle(StageStyle.UTILITY);		
		
		stage.setScene(createErrorDialog(infoTxt));
		stage.setTitle("Fehler");
		stage.show();
		
	}
	
	private static Scene createErrorDialog(String infoTxt){
		
		VBox root = new VBox();
		Scene scene = new Scene(root, 400, 140);
		
		Label infoLabel = new Label(infoTxt);
		Button okButton = new Button("OK");
		okButton.setPrefSize(100, 20);
		
		VBox.setMargin(infoLabel, new Insets(10, 10, 10, 10));
		VBox.setMargin(okButton, new Insets(25, 0, 0, scene.getWidth() / 2 - okButton.getPrefWidth() / 2));
		
		root.getChildren().addAll(infoLabel, okButton);
		
		okButton.setOnAction((e) -> {
			stage.close();
		});
		
		return scene;
		
	}
	
}
