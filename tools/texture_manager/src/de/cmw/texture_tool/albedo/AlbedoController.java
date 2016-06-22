package de.cmw.texture_tool.albedo;

import java.util.ArrayList;

import de.cmw.texture_tool.structs.ImageObject;
import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.CheckBox;
import javafx.scene.control.Label;
import javafx.scene.control.ListView;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Slider;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.image.ImageView;
import javafx.scene.text.Font;

public class AlbedoController {

	private static AlbedoController instance;
	@SuppressWarnings("unused")
	private Scene scene;
	
	private ArrayList<Node> nodes;
	
	private boolean initialized;
	
	// FXML Elements	
	@FXML
	ListView<ImageObject> listView;
	
	@FXML
	ImageView imgView;
	
	@FXML
	Label fileName, filePath, sizeInMeters;
	
	@FXML
	TextField imgName;
	
	@FXML
	TextArea consoleArea;
	
	@FXML
	CheckBox imgAlpha, isDeco;
	
	@FXML
	Button exportBtn, deleteBtn, consoleBtn, exportConsoleBtn, imgSave;
	
	@FXML
	ScrollPane consoleScrollPane;
	
	@FXML
	Slider meter_slider;
	
	
	public AlbedoController(){
		if(instance == null){
			instance = this;
			initialized = false;
		}		
		nodes = new ArrayList<>();				
	}
	
	
	@SuppressWarnings("unchecked")
	public void init(Scene scene){
		
		this.scene = scene;
		
		if(!initialized){
							
			nodes.add(listView = (ListView<ImageObject>)scene.lookup("#listView"));
			nodes.add(fileName = (Label)scene.lookup("#fileName"));
			nodes.add(filePath = (Label)scene.lookup("#filePath"));
			nodes.add(sizeInMeters = (Label)scene.lookup("#size_in_meters"));
			nodes.add(imgName = (TextField)scene.lookup("#imgName"));
			nodes.add(imgAlpha = (CheckBox)scene.lookup("#imgAlpha"));
			nodes.add(isDeco = (CheckBox)scene.lookup("#isDeco"));
			nodes.add(exportBtn = (Button)scene.lookup("#exportBtn"));
			nodes.add(deleteBtn = (Button)scene.lookup("#deleteBtn"));
			nodes.add(consoleBtn = (Button)scene.lookup("#consoleBtn"));
			nodes.add(exportConsoleBtn = (Button)scene.lookup("#exportConsoleBtn"));
			nodes.add(imgSave = (Button)scene.lookup("#imgSave"));
			nodes.add(consoleScrollPane = (ScrollPane)scene.lookup("#consoleScrollPane"));
			nodes.add(meter_slider = (Slider)scene.lookup("#meter_slider"));
			
			// Elements in Containers can only be found later after initialization of container
			Platform.runLater(new Runnable() {				
				@Override
				public void run() {					
					nodes.add(imgView = (ImageView)scene.lookup("#imgView"));
					nodes.add(consoleArea = (TextArea)scene.lookup("#consoleArea"));			
					
					for(Node cur : nodes){
						checkRef(cur, "");
					}
				}
			});
			
			Label placeHolder = new Label("Bilddateien hier ablegen");
			placeHolder.setFont(new Font("Arial", 32));
			listView.setPlaceholder(placeHolder);
			
			initialized = true;			
			
		}
		
	}
	
	
	private void checkRef(Node n, String name){
		System.out.println(name + ": " + n);
	}
	
	
	public static AlbedoController getInstance(){
		return instance;
	}
	
}
