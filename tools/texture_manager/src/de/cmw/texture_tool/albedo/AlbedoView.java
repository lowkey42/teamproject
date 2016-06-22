package de.cmw.texture_tool.albedo;

import java.io.IOException;

import de.cmw.texture_tool.structs.ImageObject;
import javafx.application.Platform;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.fxml.FXMLLoader;
import javafx.scene.image.Image;
import javafx.scene.layout.Pane;

public class AlbedoView {

	private Pane root;
	
	private AlbedoPresenter presenter;
	private AlbedoController controller;
	
	public AlbedoView(){
		initView();
	}
	
	private void initView(){
							
		try {
			root = FXMLLoader.load(getClass().getResource("Albedo.fxml"));
		} catch (IOException e) {
			e.printStackTrace();
		}				

		controller = AlbedoController.getInstance();
		
		// Settings for the TextArea of Console
		controller.consoleArea.setEditable(false);
		showConsole(false);
		
		// Settings for the Image View (max Width = 1024)
		controller.imgView.setFitWidth(1024);
		
		// Set Listener method invocations for checkboxes
		controller.isDeco.setOnAction((e) -> { presenter.onClick(e); });
		controller.imgAlpha.setOnAction((e) -> { presenter.onClick(e); });
		
		// Set Listener method invocations for buttons
		controller.exportBtn.setOnAction((e) -> { presenter.onClick(e); });
		controller.deleteBtn.setOnAction((e) -> { presenter.onClick(e); });
		controller.consoleBtn.setOnAction((e) -> { presenter.onClick(e); });
		controller.exportConsoleBtn.setOnAction((e) -> {presenter.onClick(e); });
		controller.imgSave.setOnAction((e) -> { presenter.onClick(e); });
		
		// Set Listener method invocations for sliders
		controller.meter_slider.valueProperty().addListener((obs, newVal, oldVal) -> {
			presenter.onSliderChange(obs, newVal, oldVal);
		});
		
		// Set Listener method invocations for Drag&Drop
		controller.listView.setOnDragEntered((e) -> { presenter.onDragEntered(e); });
		controller.listView.setOnDragOver((e) -> { presenter.onDragOver(e); });
		controller.listView.setOnDragExited((e) -> { presenter.onDragExited(e); });
		controller.listView.setOnDragDropped((e) -> { presenter.onDragDropped(e); });
		
		// Set Listener method invocations for ListView-Item-Selection
		controller.listView.getSelectionModel().selectedItemProperty().addListener((obs, oldImgObj, newImgObj) -> {
			presenter.onListItemSelected(newImgObj);
		});
		
	}
	
	public void setPresenter(AlbedoPresenter pres){
		presenter = pres;
	}
	
	protected void setImgObjName(String name){
		controller.imgName.setText(name);
	}
	
	protected void setAlphaStatus(boolean alpha){
		controller.imgAlpha.setSelected(alpha);
	}
	
	protected void setDecoStatus(boolean deco){
		controller.isDeco.setSelected(deco);
	}
	
	protected void setFileName(String str){
		controller.fileName.setText(str);
	}
	
	protected void setPathName(String str){
		controller.filePath.setText(str);
	}
	
	protected void setSizeInMeters(String str){
		controller.sizeInMeters.setText(str);
	}
	
	protected void setMeterSliderValue(double val){
		controller.meter_slider.setValue(val);
	}
	
	protected void disableDependentUI(boolean bool){
		controller.imgName.setDisable(bool);
		controller.imgAlpha.setDisable(bool);
		controller.isDeco.setDisable(bool);
		controller.meter_slider.setDisable(bool);
		controller.imgSave.setDisable(bool);
		if(bool)
			controller.sizeInMeters.setText("Kein Objekt ausgewählt");
	}
	
	protected void setImage(Image img){
		controller.imgView.setImage(img);
		controller.imgView.setFitHeight(img.getHeight());
	}
	
	protected boolean getAlpha() {
		return controller.imgAlpha.isSelected();
	}
	
	protected boolean getDecoration() {
		return controller.isDeco.isSelected();
	}
	
	protected double getMeterSliderValue(){
		return controller.meter_slider.getValue();
	}
	
	protected String getImgName(){
		return controller.imgName.getText();
	}
	
	protected ImageObject getSelectedItem(){
		return controller.listView.getSelectionModel().getSelectedItem();
	}
	
	protected int getSelectedItemIndex(){
		return controller.listView.getSelectionModel().getSelectedIndex();
	}
	
	protected void bindConsole(ObservableValue<? extends String> obs){
		controller.consoleArea.textProperty().bind(obs);
	}
	
	protected void setSelectedItem(int index){		
		// need to be executed later not directly with click on another element
		Platform.runLater(new Runnable() {			
			@Override
			public void run() {
				controller.listView.getSelectionModel().select(index);
			}
		});		
	}
	
	protected void showConsole(boolean b){
		controller.consoleScrollPane.setVisible(b);
		controller.consoleBtn.setVisible(b);
		controller.exportConsoleBtn.setVisible(b);
	}
	
	protected ObservableList<ImageObject> getList(){
		return controller.listView.getItems();
	}
	
	protected void clearEntries(){
		controller.fileName.setText("Dateiname: ");
		controller.filePath.setText("Dateipfad: ");
		controller.imgName.setText("");
		controller.imgAlpha.setSelected(false);
		controller.imgView.setImage(null);
	}
	
	public Pane get(){
		return root;
	}
	
}
