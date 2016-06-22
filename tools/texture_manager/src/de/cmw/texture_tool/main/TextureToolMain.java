package de.cmw.texture_tool.main;

import java.io.File;

import de.cmw.texture_tool.albedo.AlbedoController;
import de.cmw.texture_tool.albedo.AlbedoPresenter;
import de.cmw.texture_tool.albedo.AlbedoView;
import de.cmw.texture_tool.normal.NormalPresenter;
import de.cmw.texture_tool.normal.NormalView;
import de.cmw.texture_tool.utils.InfoDialog;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class TextureToolMain extends Application {

	private static Stage stage;
	private AlbedoController controller;
	
	@Override
	public void start(Stage primaryStage) throws Exception {

		stage = primaryStage;
		
		MainPresenter pres = onInit();
		Scene scene = new Scene(pres.getView());	
		
		// only needed for AlbedoController, it's the first shown view when starting
		controller = AlbedoController.getInstance();	
		controller.init(scene);
		Platform.runLater(() -> {	
			checkPath();
		});
				
		primaryStage.setTitle("Into The Light - Texture JSON Tool");
		primaryStage.setScene(scene);
		primaryStage.setResizable(false);
		primaryStage.show();
		
	}

	private MainPresenter onInit(){
		
		MainPresenter pres = new MainPresenter();
		AlbedoPresenter albPres = new AlbedoPresenter();
		NormalPresenter normalPres = new NormalPresenter();
		MainView view = new MainView();
		AlbedoView albView = new AlbedoView();
		NormalView normalView = new NormalView();
		Model model = new Model();
		
		pres.setModel(model);
		albPres.setModel(model);
		normalPres.setModel(model);
		view.setPresenter(pres);
		albView.setPresenter(albPres);
		normalView.setPresenter(normalPres);
		pres.setAlbedoPresenter(albPres);
		pres.setNormalPresenter(normalPres);
		pres.setView(view);	
		albPres.setView(albView);
		normalPres.setView(normalView);
		pres.showAlbedoView();
		
		return pres; 
		
	}
	
	
	public static void main(String[] args) {
		launch(args);
	}
	
	
	protected static void closeApplication(){
		if(stage != null){
			stage.close();
		}
	}
	
	private void checkPath(){		
		System.out.println(System.getProperty("user.dir"));
		File paths[] = { new File("assets"), new File ("assets_ext") };
		if (!paths[0].exists() || !paths[1].exists()){
			String infoTxt = "Die Anwendung befindet sich nicht im entsprechenden Verzeichnis\n"
					   		 + "Das Tool muss im selben Ordner wie assets & assets_ext liegen\n"
					   		 + "Die Anwendung wird jetzt geschlossen";
			InfoDialog.showErrorDialog(infoTxt);
			stage.close();
		}		
	}

}
