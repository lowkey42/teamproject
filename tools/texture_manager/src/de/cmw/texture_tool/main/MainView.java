package de.cmw.texture_tool.main;

import javafx.scene.control.CheckMenuItem;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuBar;
import javafx.scene.control.MenuItem;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;;

public class MainView extends VBox {

	private Pane root;
	
	private MainPresenter presenter;
	
	private CheckMenuItem consoleItem;
	
	public MainView(){
		
		initView();
		
	}
	
	private void initView(){
		
		MenuBar menuBar = new MenuBar();
		
		Menu fileMenu = new Menu("Datei");
		Menu viewMenu = new Menu("Ansicht");
		Menu helpMenu = new Menu("Info");
		
		MenuItem closeItem = new MenuItem("Schließen");
		MenuItem albedoItem = new MenuItem("Albedo");
		MenuItem normalItem = new MenuItem("Normal");
		consoleItem = new CheckMenuItem("Konsole anzeigen");
		MenuItem helpItem = new MenuItem("Hilfe");
		MenuItem aboutItem = new MenuItem("Über");
		
		fileMenu.getItems().add(closeItem);
		viewMenu.getItems().add(albedoItem);
		viewMenu.getItems().add(normalItem);
		viewMenu.getItems().add(consoleItem);
		helpMenu.getItems().add(helpItem);
		helpMenu.getItems().add(aboutItem);
		
		menuBar.getMenus().addAll(fileMenu, viewMenu, helpMenu);
		
		// Attach Listeners to Methods in Presenter-Class
		closeItem.setOnAction((e) -> { presenter.onClose(e); });
		albedoItem.setOnAction((e) -> { presenter.onAlbedo(e); });
		normalItem.setOnAction((e) -> { presenter.onNormal(e); });
		consoleItem.setOnAction((e) -> { presenter.onConsole(e); });
		helpItem.setOnAction((e) -> { presenter.onHelp(e); });
		aboutItem.setOnAction((e) -> { presenter.onAbout(e); });
		
		setPrefSize(200, 200);
		getChildren().add(menuBar);
		
	}
	
	public void setPresenter(MainPresenter pres){
		presenter = pres;
	}
		
	public void setContent(Pane p){
		if(root != null){
			getChildren().remove(root);
		}
		root = p;
		getChildren().add(root);
	}
	
	public boolean getConsoleSelection(){
		return consoleItem.isSelected();
	}
	
}
