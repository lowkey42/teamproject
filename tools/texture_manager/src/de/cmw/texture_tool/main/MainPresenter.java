package de.cmw.texture_tool.main;

import java.util.ArrayList;

import de.cmw.texture_tool.Presentator;
import de.cmw.texture_tool.albedo.AlbedoPresenter;
import de.cmw.texture_tool.normal.NormalPresenter;
import javafx.event.ActionEvent;
import javafx.scene.layout.Pane;

public class MainPresenter {

	private MainView view;
	@SuppressWarnings("unused")
	private Model model;
	
	private AlbedoPresenter albedoPresenter;
	private NormalPresenter normalPresenter;
	
	private ArrayList<Presentator> subpresenters;
	
	public MainPresenter(){
		subpresenters = new ArrayList<>();
	}
		
	public void setView(MainView v){
		view = v;
	}
	
	public void setModel(Model m){
		model = m;
	}
	
	public void setAlbedoPresenter(AlbedoPresenter pres){		
		albedoPresenter = pres;
		subpresenters.add(pres);
	}
	
	public void setNormalPresenter(NormalPresenter pres){
		normalPresenter = pres;
		subpresenters.add(pres);
	}
	
	public Pane getView(){
		return view;
	}
	
	// Show different Views
	public void showAlbedoView(){		
		view.setContent(albedoPresenter.getView());
	}
	
	public void showNormalView(){
		view.setContent(normalPresenter.getView());
	}
	
	// Methods called by View-Listeners		
	protected void onClose(ActionEvent e){
		TextureToolMain.closeApplication();
	}

	protected void onAlbedo(ActionEvent e) {
		showAlbedoView();
	}
	
	public void onNormal(ActionEvent e) {
		showNormalView();		
	}

	protected void onHelp(ActionEvent e) {
		System.out.println("onHelp triggered!");
	}	
	
	protected void onAbout(ActionEvent e){
		AboutDialog.showDialog();
	}

	protected void onConsole(ActionEvent e) {
		for(Presentator cur : subpresenters){
			cur.onConsoleTrigger(e);
		}
	}
	
}
