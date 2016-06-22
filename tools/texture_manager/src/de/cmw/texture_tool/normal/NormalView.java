package de.cmw.texture_tool.normal;

import java.io.IOException;

import javafx.fxml.FXMLLoader;
import javafx.scene.layout.Pane;

public class NormalView {

	private Pane root;
	
	private NormalPresenter presenter;
	private NormalController controller;
	
	public NormalView(){
		initView();
	}
	
	private void initView(){
		
		try {
			root = FXMLLoader.load(getClass().getResource("Normal.fxml"));
		} catch (IOException e) {
			e.printStackTrace();
		}	
		
	}
	
	public void setPresenter(NormalPresenter pres){
		presenter = pres;
	}
	
	public Pane get(){
		return root;
	}
	
}
