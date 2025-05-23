package ai.vespa.schemals.lsp.common.completion;

import java.util.List;

import org.eclipse.lsp4j.CompletionItem;

import ai.vespa.schemals.context.EventCompletionContext;

public interface CompletionProvider {
    public List<CompletionItem> getCompletionItems(EventCompletionContext context);
}
