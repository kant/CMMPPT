/*
 * Created on Nov 24, 2004
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package witviz;


/**
 * @author gresh
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class WitBomUpperFragmentEdge extends WitEdge {
    private int bomEntry;
	private WitNode operation;
	private WitNodeInvisible invisible;
	public WitBomUpperFragmentEdge() {
		super();
		edgeType = "bomupper";
    }
	public String getNiceName() {
		String key = "BOM: Operation: "+operation.getText()+ " Part: " + invisible.getPart().getText() + " BomEntry: "+ bomEntry;
        return key;
	}
	public int getBomEntry(){
		return bomEntry;
	}
	public void setBomEntry(int b){
		bomEntry = b;
	}
	public void setOperation(WitNode o){
		operation = o;
	}
	public WitNode getOperation() {
		return operation;
	}
	public void setInvisible(WitNodeInvisible p) {
		invisible = p;
	}
	public WitNode getInvisible() {
		return invisible;
	}
	public String createKey() {
		String key;
		String op,in;
		op = operation.getText();
		in = invisible.createKey();
		key = createKey(op,in,bomEntry);
		text = key;
	    return key;
	}
	public static String createKey(String op, String in, int entry) {
		String key;
		key = "Operation: " + op + "Invisible: "+ in + " bomEntry: " + entry;
	    return key;
	}
	public String getSource() {
		return operation.getText();
	}
	public String getTarget() {
		return invisible.getText();
	}
	
	

}
