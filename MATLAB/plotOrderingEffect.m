function [] = plotOrderingEffect()
    temp = [66	81	57	68	132	65	55	53	67	70	71.4000000000000
    64	80	56	66	130	61	54	54	65	69	69.9000000000000
    61	78	55	66	131	62	53	54	63	70	69.3000000000000];
    f_Ordering_effect = figure("Name", "Ordering_effect");
    hold on
    plot(0 : 10, temp(1, :), '.', 'MarkerSize', 20, 'DisplayName', "No ordering");
    plot(0 : 10, temp(2, :), '.', 'MarkerSize', 20, 'DisplayName', "Ordering 1");
    plot(0 : 10, temp(3, :), '.', 'MarkerSize', 20, 'DisplayName', "Ordering 2");
    xticklabels({"Data set 1", "Data set 2", "Data set 3", "Data set 4", "Data set 5", "Data set 6", ...
        "Data set 7", "Data set 8", "Data set 9", "Data set 10", "Average"})
    xticks(0:10)
    ylabel("Number of added edges")
    legend('box', 'off', 'FontSize', 12)
    ax = gca;
    ax.XAxis.FontSize = 12;
    ax.YAxis.FontSize = 12;
    ax.XLabel.FontSize = 15;
    ax.YLabel.FontSize = 15;
    set(gcf, 'Position', [0, 0, 1250, 800]);
    exportgraphics(f_Ordering_effect, "figure/f_Ordering_effect.png", 'Resolution', 600);
end
