function [BusinessAllocation] = plotOverview(edgeStat)
    load flatNodeArray.mat
    BusinessAllocation = figure("Name", "Edge Statistic");
    ax = axes('Parent', BusinessAllocation);
    for k = 1 : 3
        maxVal = max(edgeStat(:, 2 + k));
        cmap = turbo(maxVal + 1); % 定义颜色映射（可以根据实际需求选择其他颜色映射）
        
        hold on
        % 绘制每条边
        for i = 1 : size(edgeStat, 1)
            % 根据权值确定颜色深浅
            colorIndex = edgeStat(i, 2 + k) + 1;
            color = cmap(colorIndex, :);
            lineWidth = 1 + 3 * ((colorIndex + 1) / (maxVal + 1));
            plot3([flatNodeArray{edgeStat(i, 1) + 1}.x, flatNodeArray{edgeStat(i, 2) + 1}.x], ...
            [flatNodeArray{edgeStat(i, 1) + 1}.y, flatNodeArray{edgeStat(i, 2) + 1}.y], ...
            k * ones(1, 2), '-o', 'Color', color, 'LineWidth', lineWidth, ...
             'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end
        colormap(cmap);
        colorbarHandle = colorbar;
        colorbarHandle.Label.String = 'Number of transactions';
        colorbarHandle.FontSize = 11;
        colorbarHandle.Label.FontSize = 14;
        clim([0, maxVal]);
        axis off
        % 启用数据提示模式
        dcm = datacursormode(gcf);
        set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
        set(gcf, 'Color', 'white');
    end
end