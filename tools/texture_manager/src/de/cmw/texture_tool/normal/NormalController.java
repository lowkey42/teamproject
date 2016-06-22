package de.cmw.texture_tool.normal;

import java.util.ArrayList;

import javafx.scene.Node;
import javafx.scene.Scene;

public class NormalController {

	private static NormalController instance;	
	@SuppressWarnings("unused")
	private Scene scene;
	
	private ArrayList<Node> nodes;
	
	private boolean initialized;
	
	public NormalController(){
		if(instance == null){
			instance = this;
			initialized = false;
		} 
		nodes = new ArrayList<>();
	}
	
	public void init(Scene scene){
		
		this.scene = scene;
		
		if(!initialized){
			
			
			
		}
		
	}
	
	private void checkRef(Node n, String name){
		System.out.println(name + ": " + n);
	}	
	
	public static NormalController getInstance(){
		return instance;
	}
	
}
