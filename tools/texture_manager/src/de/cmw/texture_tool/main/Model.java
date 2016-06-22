package de.cmw.texture_tool.main;

import java.util.ArrayList;

import de.cmw.texture_tool.structs.ImageObject;

public class Model {

	private ArrayList<ImageObject> imgObjects;
	private ImageObject curSelection;
	
	public Model() {
		
		imgObjects = new ArrayList<>();
		
	}
	
	public ArrayList<ImageObject> getObjects(){
		return imgObjects;
	}
	
	public void addEntry(ImageObject obj){
		imgObjects.add(obj);
	}
	
	public void removeEntry(ImageObject obj){
		int index = imgObjects.indexOf(obj);
		if(index >= 0){
			imgObjects.remove(index);
			System.out.println("object succesfully removed!");
		}
	}
	
	public void setCurrentSelection(ImageObject obj){
		curSelection = obj;
	}
	
	public int getCurSelectionIndex(){
		return imgObjects.indexOf(curSelection);
	}
	
}
