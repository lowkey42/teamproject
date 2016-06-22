package de.cmw.texture_tool.normal;

import de.cmw.texture_tool.Presentator;
import de.cmw.texture_tool.main.MainPresenter;
import de.cmw.texture_tool.main.Model;
import javafx.event.ActionEvent;
import javafx.scene.layout.Pane;

public class NormalPresenter implements Presentator {

	private MainPresenter mpres;
	
	private Model model;
	private NormalView view;
	
	public NormalPresenter(){
		
	}
	
	public void setPresenter(MainPresenter p){
		mpres = p;
	}
	
	public void setModel(Model m){
		model = m;
	}
	
	public void setView(NormalView v){
		view = v;
		
		// view.bindConsole;
	}
	
	public Pane getView(){
		return view.get();
	}

	@Override
	public void onConsoleTrigger(ActionEvent e) {
		// TODO Auto-generated method stub		
	}
	
}
