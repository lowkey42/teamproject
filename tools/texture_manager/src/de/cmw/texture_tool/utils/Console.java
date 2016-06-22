package de.cmw.texture_tool.utils;

import java.util.Calendar;

import javafx.beans.property.SimpleStringProperty;
import javafx.beans.value.ObservableValue;

public class Console {

	private static StringBuilder content = new StringBuilder();
	private static SimpleStringProperty strProp = new SimpleStringProperty();
	
	public static String getContent(){
		return content.toString();
	}
	
	public static ObservableValue<? extends String> getProp(){
		return strProp;
	}
	
	public static void log(String str){
		content.insert(0, "LOG: " + str + "\n");
		syncProp();
	}
	
	public static void warn(String str){
		// TODO
		content.insert(0, "WARN: " + str + "\n");
	}
	
	public static void error(String str){
		// TODO
		content.insert(0, "ERROR: " + str + "\n");
	}
	
	public static void clear(){
		content.setLength(0);
		syncProp();
	}
	
	@SuppressWarnings("unused")
	private static String getTime(){
		Calendar cal = Calendar.getInstance();
		return cal.get(Calendar.HOUR_OF_DAY) + ":" + cal.get(Calendar.MINUTE) + ":" + cal.get(Calendar.SECOND);
	}
	
	private static void syncProp(){
		strProp.set(content.toString());
	}
	
}
