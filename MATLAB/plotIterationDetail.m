function [f_iterationDetail] = plotIterationDetail()
    f_iterationDetail = figure("Name", "iterationDetail");
    set(gcf, 'WindowState', 'maximized');
    tiledlayout(3, 4, 'TileSpacing', 'tight')
    
    for i = 1 : 10
        nexttile
        eval("iterStatistic = readmatrix('output\" + num2str(i) + "iterStatistic.txt');");
        n_iter = size(iterStatistic, 1);
        ax = gca;
        ax.XAxis.Exponent = 3;
        xlabel("Epoch", 'FontSize', 12)
        ax = gca;
        yyaxis left;
        plot(1 : n_iter, iterStatistic(:, 3), 'LineWidth', 1, 'DisplayName', "Added edges")
        ylabel("Number of added edges", 'FontSize', 12)
        yyaxis right;
        plot(1 : n_iter, iterStatistic(:, 4), 'LineWidth', 1, 'DisplayName', "Used piles")
        ax.YAxis(2).Exponent = 3;
        ytickformat('%.1f')
        ylabel("Number of used piles", 'FontSize', 12)
        legend('box', 'off')
        title("Data set " + num2str(i), 'FontSize', 12);
        
    end
end