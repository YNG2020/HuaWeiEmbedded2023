function [BusNumInMiltiEdge] = plotBusNumInMiltiEdge(edgeStat)
    BusNumInMiltiEdge = figure("Name", "Business Number In Different Multiple Edge");
    plot(1 : max(edgeStat, 4), edgeStat(:, 6 : 6 + max(edgeStat, 4) - 1), '--+')
    xticks(1 : max(edgeStat, 4))
    xlabel("Multiple edge ID", 'FontSize', 13)
    ylabel("Business number", 'FontSize', 13)
end
