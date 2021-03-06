package net.sourceforge.ufoai.ufoscripteditor.parser.parsers.subparser.ui;

import net.sourceforge.ufoai.ufoscripteditor.parser.IParserContext;
import net.sourceforge.ufoai.ufoscripteditor.parser.parsers.subparser.IUFOSubParser;
import net.sourceforge.ufoai.ufoscripteditor.parser.parsers.subparser.IUFOSubParserFactory;

public class AbstractValueSubParser extends AbstractNodeSubParser {
	public static final IUFOSubParserFactory FACTORY = new UFONodeParserFactoryAdapter() {
		@Override
		public String getID() {
			return "abstractvalue";
		}

		@Override
		public IUFOSubParser create(IParserContext ctx) {
			return new AbstractValueSubParser(ctx);
		}

	};

	@Override
	public IUFOSubParserFactory getNodeSubParserFactory() {
		return FACTORY;
	}

	public AbstractValueSubParser(IParserContext ctx) {
		super(ctx);
		// IUFOSubParserFactory factory = getNodeSubParserFactory();
		// Registration for event properties

	}
}
