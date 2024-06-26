function [TranNumInMiltiEdge] = plotTranNumInMultiEdge(edgeStat)
    TranNumInMiltiEdge = figure("Name", "Transaction Number In Different Multiple Edge");
    plot(1 : max(edgeStat(:, 4)), edgeStat(:, 6 : 6 + max(edgeStat(:, 4)) - 1), '-+')
    xticks(1 : max(edgeStat(:, 4)))
    xlabel("Multiple edge ID", 'FontSize', 13)
    ylabel("Transaction number", 'FontSize', 13)
    title("Strategy step 6 (Iteration-plus)", 'FontSize', 13)
end
