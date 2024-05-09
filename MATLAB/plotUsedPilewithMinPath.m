function [singleTransaction, figureUsedPile] = ...
    plotUsedPilewithMinPath(transactionID, transactionMinPath, transactionPassEdgeID, edgeStat, edgePile)
    transactionID = transactionID + 1;
    edgePassed1 = transactionMinPath{transactionID};
    [UsedPile1, ax1] = plotUsedPile(edgePile, edgePassed1);
    edgePassed2 = transactionPassEdgeID{transactionID};
    [UsedPile2, ax2] = plotUsedPile(edgePile, edgePassed2);
    [singleTransaction] = plotSingleTransaction(edgeStat, edgePile, edgePassed1, edgePassed2);
    
    figureUsedPile = figure("Name", "figureUsedPile");
    set(gcf, 'WindowState', 'maximized');
    s1 = subplot(1,2,1);
    s1.XTick = ax1.XTick;
    s1.YTick = ax1.YTick;
    s1.XTickLabel = ax1.XTickLabel;
    s1.YTickLabel = ax1.YTickLabel;
    s1.XLim = ax1.XLim;
    s1.YLim = ax1.YLim;
    s1.XLabel = ax1.XLabel;
    s1.YLabel = ax1.YLabel;
    title("业务" + num2str(transactionID) + "在不考虑通道是否被其它业务占据时分配的业务路径上的通道占用情况")

    s2 = subplot(1,2,2);
    s2.XTick = ax2.XTick;
    s2.YTick = ax2.YTick;
    s2.XTickLabel = ax2.XTickLabel;
    s2.YTickLabel = ax2.YTickLabel;
    s2.XLim = ax2.XLim;
    s2.YLim = ax2.YLim;
    s2.XLabel = ax2.XLabel;
    s2.YLabel = ax2.YLabel;
    title("业务" + num2str(transactionID) + "考虑通道被其它业务占据时分配的业务路径上的通道占用情况")

    fig1 = get(ax1, 'children'); % get handle to all the children in the figure
    fig2 = get(ax2, 'children');
    copyobj(fig1,s1); % copyobj creates copies of graphics objects and assigns the objects to the new parent.
    copyobj(fig2,s2); % copyobj does not copy properties
    close(UsedPile1);
    close(UsedPile2);

end