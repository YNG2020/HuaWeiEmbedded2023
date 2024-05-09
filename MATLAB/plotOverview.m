function [TransactionAllocation] = plotOverview(edgeStat)
    load flatNodeArray.mat flatNodeArray
    load flatNodeArray.mat nodeArray
    load flatNodeArray.mat args

    TransactionAllocation = figure("Name", "Edge Statistic");
    % screen_size = get(groot, 'ScreenSize');
    % window_position = [screen_size(1) + 0 * screen_size(3), screen_size(2) + 0 * screen_size(4)];
    % window_size = [1.0 * screen_size(3), 1.0 * screen_size(4)];
    % set(TransactionAllocation, 'Position', [window_position, window_size]);
    set(gcf, 'WindowState', 'maximized');
    tiledlayout(2, 2, "TileSpacing", "none");

    tranStatistic = zeros(size(edgeStat, 1), 3);
    fileID = fopen('transactionStatistic.txt', 'r');
    for i = 1 : size(edgeStat, 1)
        tranStatistic(i, :) = fscanf(fileID, '%d', 3);
    end

    for k = 1 : 4 
        nexttile
        if k >= 2
            maxVal = max(edgeStat(:, 2 + k - 1));
        else
            maxVal = max(tranStatistic(:, 3));
        end
        cmap = turbo(maxVal + 1); % 定义颜色映射（可以根据实际需求选择其他颜色映射）
        hold on
        % 绘制每条边
        for i = 1 : size(edgeStat, 1)
            % 根据权值确定颜色深浅
            if k >= 2
                colorIndex = edgeStat(i, 2 + k - 1) + 1;
            else
                colorIndex = tranStatistic(i, 3) + 1;
            end
            color = cmap(colorIndex, :);
            lineWidth = 1 + 3 * ((colorIndex + 1) / (maxVal + 1));
            plot3([flatNodeArray{edgeStat(i, 1) + 1}.x, flatNodeArray{edgeStat(i, 2) + 1}.x], ...
            [flatNodeArray{edgeStat(i, 1) + 1}.y, flatNodeArray{edgeStat(i, 2) + 1}.y], ...
            [k k], '-o', 'Color', color, 'LineWidth', lineWidth, ...
             'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end
        colormap(cmap);
        colorbarHandle = colorbar;
        colorbarHandle.FontSize = 11;
        colorbarHandle.Label.FontSize = 14;
        clim([0, maxVal]);
        if k == 1
            colorbarHandle.Location = 'westoutside';
            colorbarHandle.Label.String = 'Number* of transactions';
            colorbarHandle.Ticks = round(colorbarHandle.Ticks);
            % colorbarHandle.Position = [0.05, 0.52, 0.01, 0.40]; % [x, y, width, height]
        elseif k == 2
            colorbarHandle.Location = 'eastoutside';
            colorbarHandle.Label.String = 'Number of transactions';
            colorbarHandle.Ticks = round(colorbarHandle.Ticks);
        elseif k == 3
            colorbarHandle.Location = 'westoutside';
            colorbarHandle.Label.String = 'Number of multiple edge';
            colorbarHandle.Ticks = round(colorbarHandle.Ticks);
            
        elseif k == 4
            colorbarHandle.Location = 'eastoutside';
            colorbarHandle.Label.String = 'Edge utilization rate';
            colorbarHandle.Ticks = round(colorbarHandle.Ticks);
        end
        axis off
        % 启用数据提示模式
        dcm = datacursormode(gcf);
        set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
        set(gcf, 'Color', 'white');
    end

    hold off
end