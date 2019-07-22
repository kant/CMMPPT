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
public class WitBopEdge extends WitEdge {
    private int bopEntry;
	private WitNode operation;
	private WitNode part;
	public WitBopEdge() {
		super();
		edgeType = "bop";
		
	}
	public void setOperation(WitNode o) {
		operation = o;
		target = operation.getText();
	}
	public WitNode getOperation() {
		return operation;
	}
	public void setPart(WitNode p) {
		part = p;
		source = part.getText();
	}
	public WitNode getPart() {
		return part;
	}
	public String getSource() {
		return part.getText();
	}
	public String getTarget() {
		return operation.getText();
	}
	public int getBopEntry() {
		return bopEntry;
	}
	public void setBopEntry(int b) {
		bopEntry = b;
	}
	public String createKey() {
		String key;
		String op, pa;
		op = operation.getText();
		pa = part.getText();
		key = createKey(op,pa,bopEntry);
		text = key;
	    return key;
	}
	public static String createKey(String op, String pa, int entry) {
		String key;
		key = "BOP: " + " Operation: " + op + " Part: " + pa + " Bop Entry: " + entry;
        return key;
	}
	public String createNiceName() {
		return createKey();
	}

}
