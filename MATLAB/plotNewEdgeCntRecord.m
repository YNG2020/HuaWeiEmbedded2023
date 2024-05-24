function [f_newEdgeCntRecord] = plotNewEdgeCntRecord()
    load newEdgeCntRecord.mat newEdgeCntRecord
    % load totUsedPileCntRecord.mat
    n_step = size(newEdgeCntRecord, 2);
    for step = 1 : n_step
        f_newEdgeCntRecord = figure("Name", "newEdgeCntRecord");
        plot(1 : step, newEdgeCntRecord(:, 1 : step), 'LineWidth', 0.5, 'LineStyle', '--', 'Marker', '.', 'MarkerSize', 15);
        
        hold on
        meanNewEdgeCntRecord = mean(newEdgeCntRecord, 1);
        strategy = {"Initial", "Specify order", "Restrain detour", "Recycle edge", "Iteration", "Iteration-plus"};
        target = plot(1 : step, meanNewEdgeCntRecord(1 : step), ...
            'LineWidth', 2, 'Color', 'red', 'Marker', '^', ...
            'MarkerSize', 6, 'MarkerFaceColor', 'g', ...
            'MarkerEdgeColor', 'g');
        legend("Data set 1", "Data set 2", "Data set 3", "Data set 4", "Data set 5", ...
            "Data set 6", "Data set 7", "Data set 8", "Data set 9", "Data set 10", 'Average', 'box', 'off', ...
            "FontSize", 12)
        xticks(1 : step);
        xlabel("Step", "FontSize", 15)
        ylabel("Number of added edges", "FontSize", 15)
        % 设置坐标轴标签的字体大小
        ax = gca;
        ax.XAxis.FontSize = 12;
        ax.YAxis.FontSize = 12;
        ax.XLabel.FontSize = 15;
        ax.YLabel.FontSize = 15;

        % 启用数据提示工具
        dcm = datacursormode(gcf);
        set(dcm, 'Enable', 'on');
        
        % 对每个数据点手动创建DataTip
        for i = 1 : step
            % 创建DataTip
            if i == step
                datatip(target,'DataIndex', i, 'Location','southwest', 'FontSize', 11);
            else
                datatip(target,'DataIndex', i, 'FontSize', 11);
            end
            target.DataTipTemplate.DataTipRows(1).Label = "Strategy: "; 
            target.DataTipTemplate.DataTipRows(1).Value = strategy; 
            target.DataTipTemplate.DataTipRows(2).Label = "Added edges: "; 
        end
        
        set(gcf, 'Position', [0, 0, 1250, 800]);
        exportgraphics(f_newEdgeCntRecord, "figure/f_newEdgeCntRecord" + num2str(step) + ".png", 'Resolution', 600);
        hold off

    end
end