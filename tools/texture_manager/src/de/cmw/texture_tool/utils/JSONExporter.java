package de.cmw.texture_tool.utils;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import de.cmw.texture_tool.structs.ImageObject;

public class JSONExporter {

	final static double ROUNDING = 100.0;
	
	// checks if the given ImageObject has already been parsed into
	// JSON. Only checks if a material was created from the given object
	// When material exists --> texture already exists --> blueprint exists
	/**
	 * Checks if the material file for the given entity-name already exists.
	 * <br>An existing material indicates the existence of an equally named
	 * <br>blueprint with attached texture-file
	 * @param obj
	 * @return true, if the material already exists
	 * <br>false, otherwise
	 */
	public static boolean exists(ImageObject obj){				
		String curPath = System.getProperty("user.dir");
		File file = new File(curPath + "/assets_ext/materials/" + obj.getName() + ".mat");
		return file.exists();		
	}
	
	// Checks if the given ImageObject has a lowercase filename
	// if not the function returns true, because the name has
	// been changed to lowercase
	/**
	 * Checks if the filename of the <i>obj</i> is lowercase.
	 * <br>If not, the filename will be changed to lowercase.
	 * @param obj
	 * @return true, if the filename was changed to lowercase
	 * <br>false, otherwise
	 * 
	 */
	public static boolean nameToLowerCase(ImageObject obj){		
		String fileName = obj.getFileName();
		String filePath = obj.getPath();
		
		boolean isLowerCase = true;
		for(char ch : fileName.toCharArray()){
			if((int)ch >= 65 && (int)ch <= 90)
				isLowerCase = false;
		}
		
		if(!isLowerCase){
			String oldFileStr = filePath + "/" + fileName;
			String newFileStr = filePath + "/" + fileName.toLowerCase();
			File oldFile = new File(oldFileStr);
			File newFile = new File(newFileStr);
			oldFile.renameTo(newFile);
			obj.setFileName(fileName.toLowerCase());
		}		
		
		return !isLowerCase;		
	}
	
	
	public static void createMaterial(ImageObject obj) throws IOException{	
		String materialTxt = "{\n"
						   + "\t\"albedo\": \"tex:textures/" + obj.getFileName() + "\",\n"
						   /*
						   + "\t\"normal\": \"\",\n"
						   + "\t\"material\": \"\",\n"
						   + "\t\"height\": \"\",\n"
						   */
						   + "\t\"alpha\": " + ((obj.isAlpha()) ? "true" : "false") + "\n"
						   + "}\n";
		
		File materialFile = new File("assets_ext/materials/" + obj.getName() + ".mat");
		BufferedWriter bw = new BufferedWriter(new FileWriter(materialFile));
		bw.write(materialTxt);
		bw.close();
		
	}
	
	
	public static void createBlueprint(ImageObject obj) throws IOException{
		
		StringBuilder builder = new StringBuilder();

		/* OLD DEFINITION OF METERS_IN_PIXEL
		 * NOW MOVED TO SLIDER-VALUE WITH MANUAL ADJUSTMENT
		 * 
		
		double meters_per_pixel = 1.0 / 100.0;
		Image curImage = obj.getImage();
		
		double width = Math.round(curImage.getWidth() * meters_per_pixel);
		double height = Math.round(curImage.getHeight() * meters_per_pixel);
		
		if(width == 0 || height == 0){
			// Custom meters-per-pixel factor for final dimensions inside the editor
			meters_per_pixel = 1.0 / 10.0;
			width = Math.round(curImage.getWidth() * meters_per_pixel);
			height = Math.round(curImage.getHeight() * meters_per_pixel);
		}
		*/
		
		// get size informations from the object
		double sizeFactor = obj.getSizeFactor();
		double width = (int)(sizeFactor * obj.getImage().getWidth() * ROUNDING) / ROUNDING;
		double height = (int)(sizeFactor * obj.getImage().getHeight() * ROUNDING) / ROUNDING;
		
		System.out.println("width = " + width);
		System.out.println("height = " + height);
		
		builder.append(		  "{\n"
						    + "\t\"Transform\":{},\n"
						    + "\t\"Sprite\": {\n"
						    + "\t\t\"material\": \"mat:" + obj.getName() + "\",\n"
						    + "\t\t\"size\": {\"x\":" + width + ", \"y\":" + height + "},\n"
						    + "\t\t\"shadowcaster\": false\n"
						    + "\t},\n"
						    + "\t\"Editor\": {\n"
						    + "\t\t\"bounds\": {\"x\":"+ width + ", \"y\":" + height +", \"z\":1}\n"
							+ "\t}");
		
		if(!obj.isDecoration()){
		builder.append(		  ",\n"
							+ "\t\"Static_body\": {\n"
						    + "\t\t\"shape\": \"polygon\"\n"
						    + "\t}\n"
						    + "}\n");
		} else {
			builder.append("\n" + "}\n");
		}
		String blueprintTxt = builder.toString();
		
		File blueprintFile = new File("assets_ext/blueprints/" + obj.getName() + ".json");
		BufferedWriter bw = new BufferedWriter(new FileWriter(blueprintFile));
		bw.write(blueprintTxt);
		bw.close();
		
	}
	
	
	public static boolean updateTextureMap(ImageObject obj) throws IOException{
		
		boolean entryExists = false;
		
		File texMap = new File("assets_ext/assets_tex.map");
		StringBuilder texMapStr = new StringBuilder();
		
		String curParsed;
		String objEntry = obj.getName() + ".mat";
		
		if(texMap.exists()){			
			BufferedReader br = new BufferedReader(new FileReader(texMap));
			while(br.ready()){
				curParsed = br.readLine();
				if(curParsed.length() >= objEntry.length()){
					if(curParsed.substring(curParsed.length() - objEntry.length()).equals(objEntry)){
						entryExists = true;
						break;
					}
				}
				texMapStr.append(curParsed+"\n");
			}					
			br.close();
			
			if(!entryExists){
				texMapStr.append("mat:" + obj.getName() + " = materials/" + objEntry + "\n");
				BufferedWriter bw = new BufferedWriter(new FileWriter(texMap));
				bw.write(texMapStr.toString());
				bw.close();	
			}							
		}				
		return entryExists;				
	}
	
			
	public static boolean updateBlueprintMap(ImageObject obj) throws IOException{
		
		boolean entryExists = false;
		
		File texMap = new File("assets_ext/assets_ext_blueprints.map");
		StringBuilder texBpStr = new StringBuilder();
		
		String curParsed;
		String objEntry = obj.getName() + ".json";
		
		if(texMap.exists()){			
			BufferedReader br = new BufferedReader(new FileReader(texMap));
			while(br.ready()){
				curParsed = br.readLine();
				if(curParsed.length() >= objEntry.length()){
					if(curParsed.substring(curParsed.length() - objEntry.length()).equals(objEntry)){
						entryExists = true;
						break;
					}
				}
				texBpStr.append(curParsed+"\n");
			}					
			br.close();
			
			if(!entryExists){
				texBpStr.append("blueprint:" + obj.getName() + " = blueprints/" + objEntry + "\n");				
				BufferedWriter bw = new BufferedWriter(new FileWriter(texMap));
				bw.write(texBpStr.toString());
				bw.close();
			}			
		}			
		return entryExists;		
	}
	
}
