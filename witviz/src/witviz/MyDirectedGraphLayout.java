/*
 * Created on Nov 19, 2004
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package witviz;

import org.eclipse.draw2d.graph.DirectedGraph;
import org.eclipse.draw2d.graph.DirectedGraphLayout;
import org.eclipse.draw2d.internal.graph.BreakCycles;
import org.eclipse.draw2d.internal.graph.HorizontalPlacement;
import org.eclipse.draw2d.internal.graph.InitialRankSolver;
import org.eclipse.draw2d.internal.graph.InvertEdges;
import org.eclipse.draw2d.internal.graph.LocalOptimizer;
import org.eclipse.draw2d.internal.graph.MinCross;
import org.eclipse.draw2d.internal.graph.PlaceEndpoints;
import org.eclipse.draw2d.internal.graph.RankAssigmentSolver;
import org.eclipse.draw2d.internal.graph.TightSpanningTreeSolver;
import org.eclipse.draw2d.internal.graph.VerticalPlacement;


/**
 * @author gresh
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MyDirectedGraphLayout extends DirectedGraphLayout {

	public void visit(DirectedGraph graph) {
		new BreakCycles()
			.visit(graph);
		new InitialRankSolver()
			.visit(graph);
		new TightSpanningTreeSolver()
			.visit(graph);
		new RankAssigmentSolver()
			.visit(graph);
		new MyPopulateRanks()
			.visit(graph);
		new VerticalPlacement()
			.visit(graph);
		new MinCross()
			.visit(graph);
		new LocalOptimizer()
			.visit(graph);
		new HorizontalPlacement()
			.visit(graph);
		new PlaceEndpoints()
			.visit(graph);
		new InvertEdges()
			.visit(graph);
	}

}
