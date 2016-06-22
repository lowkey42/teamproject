package de.cmw.texture_tool.structs;

import java.io.File;

import de.cmw.texture_tool.utils.Console;
import javafx.scene.image.Image;

public class ImageObject {

	// Members
	private String filePath;
	private String fileName;
	// name = Presentation Name for JSON-Files
	private String name;
	private double sizeFactor;
	private boolean alpha;
	private boolean decoration;
	
	private Image image;
	
	// Constructors	
	public ImageObject(String fpath, String fname, boolean alpha, boolean decoration){
		filePath = fpath;
		fileName = fname;
		this.alpha = alpha;
		this.decoration = decoration;
		createName();
		loadImage();
	}	
	
	public ImageObject(String rpath, String fname){
		this(rpath, fname, false, false);
	}	
	
	// Getters	
	public String getPath() {
		return filePath;
	}

	public String getFileName() {
		return fileName;
	}

	public String getName() {
		return name;
	}

	public boolean isAlpha() {
		return alpha;
	}
	
	public boolean isDecoration() {
		return decoration;
	}
	
	public Image getImage(){
		return image;
	}	
	
	public double getSizeFactor(){
		return sizeFactor;
	}
	
	// Setters
	public void setFileName(String lowerCaseFilename){
		fileName = lowerCaseFilename;
	}
	
	// Sets all necessary attributes describing the structure for JSON-Export
	public void setContent(String name, boolean alpha, boolean decoration, double sizeFactor){
	
		// only change name if the new name is not equal to the existing one
		if(!name.equals(this.name)){		
			StringBuilder newName = new StringBuilder();
			boolean upperCase = false, specialSigns = false;
		
			// Testing for special letters and UpperCase characters
			for(int i = 0; i < name.length(); i++){
				char c = name.charAt(i);
				int ch = (int)(name.charAt(i));
			
					// Check if the char is UpperCase
					if(ch >= 65 && ch <= 90){
						upperCase = true;
						newName.append(Character.toLowerCase(c));
				
				// Check if the char is a special sign (any sign that is not + , - _ and 0 to 9)
				// special signs will be omitted in the final name
				} else if (!(ch >= 97 && ch <= 122) && !(ch >= 43 && ch <= 45) && !(ch >= 48 && ch <= 57) && (ch != 95)){
					specialSigns = true;
				
				} else {
					newName.append(c);
				}
			}
			
			if(upperCase || specialSigns){
				if(upperCase)
					Console.warn(name + " is uppercase!");
				if(specialSigns)
					Console.warn(name + " has special signs!");
				Console.warn("changed " + name + " TO " + newName.toString());
			}
			
			Console.log("changed name from " + this.name + " TO " + newName);
			
			this.name = newName.toString();
		
		}	
		
		setSizeFactor(sizeFactor);
		
		this.alpha = alpha;
		this.decoration = decoration;
	}
	
	
	// Methods
	private void setSizeFactor(double factor){
		sizeFactor = factor;
	}
	
	private void createName(){		
		int length = fileName.length();
		name = (fileName.charAt(length - 4) == '.') ? 
				fileName.substring(0, length - 4) : 
				fileName.substring(0, length - 5) ;
		name = name.toLowerCase();
	}
	
	private void loadImage(){		
		//System.out.println("toLoad: " + relativePath + "/" + fileName);
		File file = new File(filePath + "/" + fileName);
		if(file.exists()){
			System.out.println(fileName + " exists");
			image = new Image(file.toURI().toString());
			setSizeFactor(0);
		}
	}
	
	// toString --> shown in listView
	@Override
	public String toString(){
		return name;
	}
	
}
