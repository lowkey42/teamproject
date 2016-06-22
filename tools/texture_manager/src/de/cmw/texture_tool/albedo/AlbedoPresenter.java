package de.cmw.texture_tool.albedo;

import java.io.File;
import java.io.IOException;

import de.cmw.texture_tool.Presentator;
import de.cmw.texture_tool.main.MainPresenter;
import de.cmw.texture_tool.main.Model;
import de.cmw.texture_tool.structs.ImageObject;
import de.cmw.texture_tool.utils.Console;
import de.cmw.texture_tool.utils.JSONExporter;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.scene.control.ButtonBase;
import javafx.scene.control.CheckMenuItem;
import javafx.scene.control.ListView;
import javafx.scene.effect.GaussianBlur;
import javafx.scene.image.Image;
import javafx.scene.input.DragEvent;
import javafx.scene.input.Dragboard;
import javafx.scene.input.TransferMode;
import javafx.scene.layout.Pane;

public class AlbedoPresenter implements Presentator {

	@SuppressWarnings("unused")
	private MainPresenter mpres;
	
	private Model model;
	private AlbedoView view;
	
	private ObservableList<ImageObject> list;
	
	// Rounding 100: 2.42587 --> 242 --> 2.42
	private final double ROUNDING = 100;
	
	public AlbedoPresenter(){
		
	}
	
	
	public void setPresenter(MainPresenter p){
		mpres = p;
	}
	
	
	public void setModel(Model m){
		model = m;
	}
	
	
	public void setView(AlbedoView v){
		view = v;
		list = view.getList();
		view.bindConsole(Console.getProp());
	}
	
	
	public Pane getView(){
		refreshList();
		int index = model.getCurSelectionIndex();
		if(index >= 0){
			view.setSelectedItem(index);
		}
		return view.get();
	}
	
	
	public void onConsoleTrigger(ActionEvent e) {
		view.showConsole(((CheckMenuItem)e.getSource()).isSelected());
	}

	
	// method for handling button onClick-Events
	public void onClick(ActionEvent e) {
		String source = ((ButtonBase)e.getSource()).getId();
		String btnName = ((ButtonBase)e.getSource()).getText();
		
		Console.log("clicked on \"" + btnName + "\" Button");
		
		int selectedIndex = 0;
		switch (source){
		
			case("exportBtn"):
				exportToJSON();
				break;
				
			case("imgSave"):
				selectedIndex = view.getSelectedItemIndex();
				// -1 --> nothing selected / no element to select
				if(selectedIndex != -1) {
					ImageObject obj = view.getSelectedItem();
					
					// Calculate ingame-size of the object with given factor by slider and round
					// Factor = 1 / (maxSize / prefMeters) --> Factor = prefMeters / maxSize
					double factor = calculateSizeFactor(obj);
				
					// Set ImageObject content (attributes)
					obj.setContent(view.getImgName(), view.getAlpha(), view.getDecoration(), factor);
					// TODO --> Update Names in List without reloading the list?
					selectedIndex = view.getSelectedItemIndex();
					refreshList();
					view.setSelectedItem(selectedIndex);
					Console.log("saved " + obj.getName() + " with following attributes:\n\t-->"
							  + "alpha: " + obj.isAlpha() + " | decoration: " + obj.isDecoration());
				}
				break;
				
			case("deleteBtn"):
				selectedIndex = view.getSelectedItemIndex();
				// -1 --> nothing selected / no element to select
				removeEntry(selectedIndex);
				break;
				
			case("consoleBtn"):
				Console.clear();
				break;
		
			default:
				
				break;
				
		}		
	}
	
	
	// method for handling slider-value-change
	// hint: Sliders can only be changed when a list element is selected
	protected void onSliderChange(ObservableValue<? extends Number> observable, Number oldValue, Number newValue){
		updateMeterText();		
	}

	
	// Methods for handling Drag&Drop-Events	
	protected void onDragEntered(DragEvent e) {
				
		System.out.println("on drag entered");
		@SuppressWarnings("unchecked")
		ListView<ImageObject> listView = ((ListView<ImageObject>)e.getSource());

		// Styles for right and wrong drop
		String right = "-fx-background-color: #DDFFCC; -fx-border-color: rgb(49, 89, 23);";
		String wrong = "-fx-background-color: #FF8080; -fx-border-color: rgb(49, 89, 23);";
		
		Dragboard db = e.getDragboard();		
		if(db.hasFiles() && db.getFiles().size() == 1){
			String curPath = System.getProperty("user.dir");
			String filePath = db.getFiles().get(0).getParentFile().getAbsolutePath();
			String fileName = db.getFiles().get(0).getName();
			String relPath = relativePath(curPath, filePath);
			String ending = fileName.substring(fileName.length() - 4);
			String longEnding = fileName.substring(fileName.length()-5);
		
			listView.setStyle(isCompatibleType(ending, longEnding, relPath) ? right : wrong);		        
		} else {
			listView.setStyle(wrong);
		}
		
		listView.setEffect(new GaussianBlur());
		
	}
	
	
	protected void onDragOver(DragEvent e) {	
		e.acceptTransferModes(TransferMode.ANY);
		// Documentation: This method signals that processing of the event
		// is complete and traversal of the event dispatch chain ends.
		e.consume();
	}
	
	
	protected void onDragDropped(DragEvent e) {
		
		Dragboard db = e.getDragboard();
		
		if(db.hasFiles() && db.getFiles().size() == 1){
			String curPath = System.getProperty("user.dir");
			String filePath = db.getFiles().get(0).getParentFile().getAbsolutePath();
			String fileName = db.getFiles().get(0).getName();
			String relPath = relativePath(curPath, filePath);
			String ending = fileName.substring(fileName.length() - 4);
			String longEnding = fileName.substring(fileName.length()-5);
			if(isCompatibleType(ending, longEnding, relPath)){
				System.out.println("Dateiname: " + fileName);
				System.out.println("Dateipfad: " + filePath);
				Console.log(fileName + " succesfully dropped into Application");
				processDroppedImage(fileName, filePath);
			}
		}
		
		System.out.println("onDragDropped");
	}
	
	
	protected void onDragExited(DragEvent e) {
		
		System.out.println("on drag exited");
		
		@SuppressWarnings("unchecked")
		ListView<ImageObject> listView = ((ListView<ImageObject>)e.getSource());
		
		listView.setStyle("-fx-background-color: #FFFFFF;"
						+ "-fx-border-color: rgb(200, 200, 200);");
		listView.setEffect(null);
		
	}

	
	// Methods for handling ListView-Item-Selected-Events
	protected void onListItemSelected(ImageObject imgObj) {
		if(imgObj != null){
			Console.log("selected Item: " + imgObj.getName());
			model.setCurrentSelection(imgObj);
			
			// Enable UI-Elements that should only be accessible when selected an item from list
			view.disableDependentUI(false);
			updateMeterText();
			
			// Set values of the UI-Elements that display informations about the object
			view.setImage(imgObj.getImage());
			view.setFileName(imgObj.getFileName());
			view.setPathName(imgObj.getPath());
			view.setImgObjName(imgObj.getName());
			view.setAlphaStatus(imgObj.isAlpha());
			view.setDecoStatus(imgObj.isDecoration());
			
			// Factor * Size = Meters
			double sizeFactor = imgObj.getSizeFactor();
			double longestEdge = Math.max(imgObj.getImage().getWidth(), imgObj.getImage().getHeight());
			view.setMeterSliderValue(longestEdge * sizeFactor);
			
			
		} else {
			Console.log("List selection lost");
			// Disable UI-Elements that should only be accessible when an item from the list is selected
			view.disableDependentUI(true);
		}
	}
	
	
	// Further Methods
	private void processDroppedImage(String fileName, String filePath){
		
		ImageObject obj = new ImageObject(filePath, fileName);
		boolean duplicate = false;
		
		for(ImageObject cur : model.getObjects()){
			if(obj.getFileName().equals(cur.getFileName()))
				duplicate = true;
		}
		
		if(!duplicate){
			model.addEntry(obj);
			Console.log(obj.getName() + " added to the list");
			refreshList();
		}
		
	}
	
	
	private void refreshList(){
		list.clear();
		list.addAll(model.getObjects());
		if(list.size() == 0){
			view.clearEntries();
		}
	}

	
	private double calculateSizeFactor(ImageObject obj){
		
		//                S                    M
		// x = (longest Edge of Image / prefered meters)
		// factor = 1 / x --> 1 / (S / M) --> M / S = factor
				
		double longestEdge = Math.max(obj.getImage().getWidth(), obj.getImage().getHeight());
		double factor = ((view.getMeterSliderValue() > 1) ? view.getMeterSliderValue() : 1) / longestEdge;		
		return factor;
		
	}
	
	
	// TODO --> is it necessary to out-source this code? 
	private void updateMeterText(){
		ImageObject obj = view.getSelectedItem();
		Image img = obj.getImage();
		double sizeFactor = calculateSizeFactor(obj);
		
		double sizeX = (int)(sizeFactor * img.getWidth() * ROUNDING) / ROUNDING;
		double sizeY = (int)(sizeFactor * img.getHeight() * ROUNDING) / ROUNDING;
		
		view.setSizeInMeters(sizeX + " x " + sizeY);
	}
	
	
	private String relativePath(String curPath, String filePath){
		return new File(curPath).toURI().relativize(new File(filePath).toURI()).toString();
	}
	
	
	private boolean isCompatibleType(String ending, String longEnding, String relativePath){
		System.out.println("relative path: " + relativePath);
		return ((ending.equals(".bmp") || ending.equals(".png") || ending.equals(".jpg") 
				|| ending.equals(".gif") || longEnding.equals(".jpeg")) && 
				(relativePath.equals("assets/textures/") || relativePath.equals("assets_ext/textures/")));
	}
	
	
	private void removeEntry(int index){
		if(index != -1) {
			ImageObject selected = view.getSelectedItem();
			Console.log(selected.getName() + " removed");
			index = view.getSelectedItemIndex();
			model.removeEntry(selected);
			view.setSelectedItem((index > 0) ? index-1 : 0);
			refreshList();
		}
	}
	
	
	private void exportToJSON(){
		boolean exportFinished = false;
				
		ImageObject selected = view.getSelectedItem();
		// Do only parse, if a list-object is selected
		if(model.getObjects().size() > 0 && selected != null){
			// check if the blueprint / entity already exists
			if(!JSONExporter.exists(selected)){
				// check if the fileName is lowerCase or not
				if(JSONExporter.nameToLowerCase(selected)){
					String fileName = selected.getFileName();
					Console.warn("Filename: " 
							+ fileName 
							+ " was UpperCase and has been changed to " + selected.getFileName());
				}
				
				try {
					JSONExporter.createMaterial(selected);
					JSONExporter.createBlueprint(selected);
					JSONExporter.updateTextureMap(selected);
					JSONExporter.updateBlueprintMap(selected);
					exportFinished = true;
				} catch (IOException e) {
					Console.error(e.getMessage());
					e.printStackTrace();
				}
				
				
			} else {
				Console.error("Entity " + selected.getName() + " existiert bereits!");
				System.err.println("Entity existiert bereits!");
			}
		}
		
		if(exportFinished){
			Console.log(selected.getName() + " successfully exported!");
			removeEntry(view.getSelectedItemIndex());
		}
		
	}
	
}
