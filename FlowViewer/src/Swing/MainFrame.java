package Swing;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableColumnModel;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableModel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.ui.RectangleEdge;

import edu.uci.ics.jung.algorithms.layout.BalloonLayout;
import edu.uci.ics.jung.algorithms.layout.CircleLayout;
import edu.uci.ics.jung.algorithms.layout.DAGLayout;
import edu.uci.ics.jung.algorithms.layout.FRLayout;
import edu.uci.ics.jung.algorithms.layout.FRLayout2;
import edu.uci.ics.jung.algorithms.layout.ISOMLayout;
import edu.uci.ics.jung.algorithms.layout.KKLayout;
import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.algorithms.layout.util.Relaxer;
import edu.uci.ics.jung.graph.DelegateForest;
import edu.uci.ics.jung.graph.DirectedSparseMultigraph;
import edu.uci.ics.jung.graph.Forest;
import edu.uci.ics.jung.graph.Graph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.graph.util.Graphs;
import edu.uci.ics.jung.visualization.VisualizationModel;
import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.control.AbstractModalGraphMouse;
import edu.uci.ics.jung.visualization.control.DefaultModalGraphMouse;
import edu.uci.ics.jung.visualization.control.ModalGraphMouse;
import edu.uci.ics.jung.visualization.control.SatelliteVisualizationViewer;
import edu.uci.ics.jung.visualization.decorators.PickableEdgePaintTransformer;
import edu.uci.ics.jung.visualization.decorators.PickableVertexPaintTransformer;

public class MainFrame extends JFrame {

	private static final long serialVersionUID = 1L;
	
	private String DBAddr;
	private String DBUser;
	private String DBPass;
	
	private JFreeChart trafficChart;
	private DefaultCategoryDataset trafficDataset;
	
	private Graph<String,String> graph;
	
	private Insets inset;

	private JPanel top; // minimap, portfilter
	private JPanel nodeGraph;
	//private VisualizationViewer<String, String> nodeGraphView;
	private VisualizationViewer<String, String> nodeGraphView;
	private JPanel nodeGraphProtocolFilter;
	
	private JPanel nodeGraphInform;
	private JScrollPane nodeGraphProtocolView;
	private SatelliteVisualizationViewer<String,String> nodeGraphMap;
	private JTable nodeTrafficTable;
	private JTable nodeGraphicTable;
	private NodeTrafficTableModel nodeTrafficTableModel;
	private TableModel nodeGraphicTableModel;
	
	private JPanel trafficGraph;

	private JButton graphVerticalSizeBt;
	private JButton graphHorizontalSizeBt;
	
	private float graphVerticalSize;

	private int pWidth;
	private int pHeight;

	public MainFrame(int width, int height, String DBAddr, String DBUser, String DBPass) {
		// TODO Auto-generated constructor stub
		this.DBAddr = DBAddr;
		this.DBUser = DBUser;
		this.DBPass = DBPass;
		
		connectDB(DBAddr, DBUser, DBPass);
		
		graphVerticalSize = 0.25f;
		
		trafficDataset = new DefaultCategoryDataset();
		trafficChart = ChartFactory.createStackedAreaChart("node name : IP", "", "kb/s", trafficDataset, PlotOrientation.VERTICAL, true, true, false);
		
		initGraph();
		initComponent();
		
		this.setSize(width, height);
		this.setVisible(true);
		
		inset = this.getInsets();
		
		pWidth = this.getWidth() - (inset.left + inset.right);
		pHeight = this.getHeight() - (inset.top + inset.bottom);
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		insertTestData();
		repaint();
	}
	
	private void initGraph()
	{
		graph = Graphs.<String,String>synchronizedDirectedGraph(new DirectedSparseMultigraph<String,String>());
	}

	private void initComponent() {
		// TODO Auto-generated method stub
		
		top = new JPanel();
		
		nodeGraph = new JPanel();
		FRLayout<String,String> graphLayout = new FRLayout<String,String>(graph);
		nodeGraphView = new VisualizationViewer<String, String>(graphLayout);
		
		//nodeGraphView.setBackground(new Color(228,247,186));
		//nodeGraphView.setBackground(new Color(178,204,255));
		nodeGraphView.setBackground(new Color(255,216,216));
		
		nodeGraphView.getRenderContext().setEdgeDrawPaintTransformer(new PickableEdgePaintTransformer<String>(nodeGraphView.getPickedEdgeState(), Color.black, Color.GREEN));
		nodeGraphView.getRenderContext().setVertexFillPaintTransformer(new PickableVertexPaintTransformer<String>(nodeGraphView.getPickedVertexState(), new Color(67,116,217), new Color(5,0,153)));
		final AbstractModalGraphMouse graphMouse = new DefaultModalGraphMouse();
		
		graphMouse.setMode(ModalGraphMouse.Mode.PICKING);
		nodeGraphView.setGraphMouse(graphMouse);
		
		nodeGraphProtocolFilter = new JPanel();
		nodeGraph.add(nodeGraphProtocolFilter);
		nodeGraph.add(nodeGraphView);
		
		
		nodeGraphInform = new JPanel();
		nodeGraphMap = new SatelliteVisualizationViewer<String,String>(nodeGraphView);
		//nodeGraphMap.getRenderContext().setEdgeDrawPaintTransformer(new PickableEdgePaintTransformer<String>(nodeGraphMap.getPickedEdgeState(), Color.black, Color.GREEN));
		nodeGraphMap.getRenderContext().setVertexFillPaintTransformer(new PickableVertexPaintTransformer<String>(nodeGraphMap.getPickedVertexState(), new Color(67,116,217), new Color(5,0,153)));
		
		
		
		nodeTrafficTableModel = new NodeTrafficTableModel();
		TableColumnModel columnModel = new DefaultTableColumnModel();
		TableColumn column = new TableColumn(0);
		column.setHeaderValue("IP");
	    columnModel.addColumn(column);
	    
	    column = new TableColumn(1);
		column.setHeaderValue("PORT");
	    columnModel.addColumn(column);
	    
	    column = new TableColumn(2);
		column.setHeaderValue("PACKETS");
	    columnModel.addColumn(column);
	    
	    column = new TableColumn(3);
		column.setHeaderValue("BYTES");
	    columnModel.addColumn(column);
	    
	    column = new TableColumn(4);
		column.setHeaderValue("LEVEL");
	    columnModel.addColumn(column);
	    
	    DefaultTableCellRenderer cellAlign = new DefaultTableCellRenderer();
	    cellAlign.setHorizontalAlignment(JLabel.RIGHT);

	    
		nodeTrafficTable = new JTable(nodeTrafficTableModel, columnModel);
		/*
	    nodeTrafficTable.getColumn("IP").setCellRenderer(cellAlign);
	    nodeTrafficTable.getColumn("IP").setPreferredWidth(90);
	    nodeTrafficTable.getColumn("PORT").setCellRenderer(cellAlign);
	    nodeTrafficTable.getColumn("PORT").setPreferredWidth(30);
	    nodeTrafficTable.getColumn("PACKETS").setCellRenderer(cellAlign);
	    nodeTrafficTable.getColumn("PACKETS").setPreferredWidth(60);
	    nodeTrafficTable.getColumn("BYTES").setCellRenderer(cellAlign);
	    nodeTrafficTable.getColumn("BYTES").setPreferredWidth(60);
	    nodeTrafficTable.getColumn("LEVEL").setCellRenderer(cellAlign);
	    nodeTrafficTable.getColumn("LEVEL").setPreferredWidth(5);
		*/
		nodeGraphicTable = new JTable(nodeGraphicTableModel);

		nodeGraphProtocolView = new JScrollPane(nodeTrafficTable);
		//nodeGraphProtocolView.setLayout(new BorderLayout());
		//nodeGraphProtocolView.add(nodeTrafficTable);

		nodeGraphInform.add(nodeGraphMap);
		nodeGraphInform.add(nodeGraphProtocolView);
		
		top.add(nodeGraph);
		top.add(nodeGraphInform);
		this.add(top);
		
		
		trafficGraph = new ChartPanel(trafficChart);
		trafficGraph.setFocusable(false);
		trafficChart.getLegend().setPosition(RectangleEdge.LEFT);
		
		this.add(trafficGraph);
		
		graphVerticalSizeBt = new JButton("::::::::△▽::::::::");
		graphVerticalSizeBt.setFont(graphVerticalSizeBt.getFont().deriveFont(10.0f));
		graphVerticalSizeBt.addMouseMotionListener(new GraphVerticalSizeMotionListener());
		graphVerticalSizeBt.addActionListener(new GraphVerticalSizeMotionListener());
		this.add(graphVerticalSizeBt);
		
		graphHorizontalSizeBt = new JButton();
		graphHorizontalSizeBt.setEnabled(false);
		top.add(graphHorizontalSizeBt);
	}

	private void connectDB(String DBAddr, String DBUser, String DBPass)
	{
	}
	
	public void paint(Graphics g)
	{
		//super.repaint();
		pWidth = this.getWidth() - (inset.left + inset.right);
		pHeight = this.getHeight() - (inset.top + inset.bottom);
		
		top.setBounds(0, 0, pWidth, (int)(pHeight * (1.0f - graphVerticalSize)));
		
		nodeGraph.setBounds(0, 0, (int)(top.getWidth() * 0.8f) - 5, top.getHeight());
		
		nodeGraphView.setBounds(0, 20, nodeGraph.getWidth(), nodeGraph.getHeight() - 20);
		nodeGraphView.getGraphLayout().setSize(nodeGraphView.getSize());
		
		
		nodeGraphInform.setBounds((int)(top.getWidth() * 0.8f), 20, (int)(top.getWidth() * 0.2f), top.getHeight() - 20);
		nodeGraphMap.setBounds(0, 0, nodeGraphInform.getWidth(), nodeGraphInform.getWidth());
		nodeGraphMap.getGraphLayout().setSize(new Dimension(nodeGraphInform.getWidth(),nodeGraphInform.getWidth()));
		
		nodeGraphProtocolView.setBounds(0, nodeGraphMap.getHeight() + 5, nodeGraphInform.getWidth(), nodeGraphInform.getHeight() - nodeGraphMap.getHeight() - 5);
		/*
		Relaxer relaxer = nodeGraphView.getModel().getRelaxer();
		if(relaxer != null) {
			relaxer.stop();
			relaxer.prerelax();
			relaxer.relax();
		}
		*/
		
		trafficGraph.setBounds(0, (int)(pHeight * (1.0f - graphVerticalSize)) + 10, pWidth, (int)(pHeight * graphVerticalSize));
		trafficGraph.repaint();
		
		graphVerticalSizeBt.setBounds(0, (int)(pHeight * (1.0f - graphVerticalSize)), pWidth, 10);
		graphVerticalSizeBt.repaint();
		
		graphHorizontalSizeBt.setBounds((int)(top.getWidth() * 0.8f) - 5, 20, 5, top.getHeight());
		graphHorizontalSizeBt.repaint();
	}

	private void insertTestData() {
		// TODO Auto-generated method stub
		trafficDataset.addValue(20, "rx", "1");
		trafficDataset.addValue(40, "rx", "2");
		trafficDataset.addValue(10, "rx", "3");
		trafficDataset.addValue(60, "rx", "4");
		trafficDataset.addValue(10, "rx", "5");
		trafficDataset.addValue(20, "rx", "6");
		trafficDataset.addValue(20, "rx", "7");
		trafficDataset.addValue(70, "rx", "8");
		trafficDataset.addValue(25, "rx", "9");
		trafficDataset.addValue(15, "rx", "10");
		trafficDataset.addValue(10, "rx", "11");
		trafficDataset.addValue(20, "rx", "12");
		

		trafficDataset.addValue(10, "tx", "1");
		trafficDataset.addValue(20, "tx", "2");
		trafficDataset.addValue(30, "tx", "3");
		trafficDataset.addValue(20, "tx", "4");
		trafficDataset.addValue(10, "tx", "5");
		trafficDataset.addValue(20, "tx", "6");
		trafficDataset.addValue(50, "tx", "7");
		trafficDataset.addValue(30, "tx", "8");
		trafficDataset.addValue(35, "tx", "9");
		trafficDataset.addValue(25, "tx", "10");
		trafficDataset.addValue(5, "tx", "11");
		trafficDataset.addValue(20, "tx", "12");
		
		trafficChart.setTitle("IP : 124.15.65.76");

		nodeTrafficTableModel.addNodeTraffic(new NodeTraffic("148.35.124.7", 80, 66357, 343654, 1));
		nodeTrafficTableModel.addNodeTraffic(new NodeTraffic("253.27.64.32", 80, 5757, 3446, 1));
		nodeTrafficTableModel.addNodeTraffic(new NodeTraffic("124.15.65.76", 22, 345, 7778, 1));
		nodeTrafficTableModel.addNodeTraffic(new NodeTraffic("125.76.43.221", 80, 453, 3456, 1));
		nodeTrafficTableModel.addNodeTraffic(new NodeTraffic("57.78.3.11", 80, 765, 26754, 1));
		nodeTrafficTableModel.addNodeTraffic(new NodeTraffic("86.34.25.22", 22, 45347, 345346, 1));
		

		graph.addVertex("Internet");
		for(int i = 1; i <= 6; i++)
		{
			graph.addVertex("Node" + 1);
			graph.addEdge("rx:node"+i, "Internet","Node"+i,EdgeType.DIRECTED);
			graph.addEdge("tx:node"+i,"Node"+i, "Internet",EdgeType.DIRECTED);
		}
	}
	
	
	private class GraphVerticalSizeMotionListener implements MouseMotionListener, ActionListener{
		@Override
		public void mouseDragged(MouseEvent e) {
			int y = e.getYOnScreen() - (getY() + inset.top);
			y = (y < 0)? 0 : ((y > pHeight)? pHeight : y);
			graphVerticalSize = 1.0f - ((float)y / (float)pHeight);
			graphVerticalSize = (graphVerticalSize < 0.1f)? 0.1f : ((graphVerticalSize > 0.8f)? 0.8f : graphVerticalSize);
			
			repaint();
		}

		@Override
		public void mouseMoved(MouseEvent e) {	}

		@Override
		public void actionPerformed(ActionEvent arg0) {
			// TODO Auto-generated method stub
			Relaxer relaxer = nodeGraphView.getModel().getRelaxer();
			if(relaxer != null) {
				relaxer.stop();
				relaxer.prerelax();
				relaxer.relax();
			}
			repaint();
		}
	}
	
	public class NodeTrafficTableModel extends AbstractTableModel{
	    
	    /**
	     * 
	     */
	    private static final long serialVersionUID = 7932826462497464190L;
	    private ArrayList<NodeTraffic> nodes;

	    public NodeTrafficTableModel(){
	    	nodes = new ArrayList<NodeTraffic>();
	    }
	    @Override
	    public int getColumnCount() {
	        return 5;
	    }

	    @Override
	    public int getRowCount() {
	        return nodes.size();
	    }
	    
	    public void addNodeTraffic(NodeTraffic page){
	        int idx = nodes.size();
	        nodes.add(page);
	        fireTableRowsInserted(idx, idx); // 반드시 호출해야한다.
	    }

	    @Override
	    public Object getValueAt(int rowIndex, int columnIndex) {
	    	NodeTraffic node = nodes.get(rowIndex);
	        switch (columnIndex) {
	        case 0 :
	            return node.getNodeName();
	        case 1 :
	            return node.getPort();
	        case 2 :
	            return node.getNumOfBytes() ;
	        case 3 :
	            return node.getNumOfPackets();
	        case 4 :
	            return node.getSecurityLevel();
	        default :
	                return "invalid";
	        }
	    }
	}
	
	public class NodeTraffic
	{
		private String nodeName;
		private int port;
		private int numOfPackets;
		private int numOfBytes;
		private int securityLevel;
		
		public NodeTraffic(String nodeName, int port, int numOfPacket, int numOfBytes, int securityLevel)
		{
			this.nodeName = nodeName;
			this.port = port;
			this.numOfPackets = numOfPacket;
			this.numOfBytes = numOfBytes;
			this.securityLevel = securityLevel;
		}
		
		public String getNodeName() {
			return nodeName;
		}
		public int getPort() {
			return port;
		}
		public int getNumOfPackets() {
			return numOfPackets;
		}
		public int getNumOfBytes() {
			return numOfBytes;
		}	
		public int getSecurityLevel()
		{
			return securityLevel;
		}
	}
}
