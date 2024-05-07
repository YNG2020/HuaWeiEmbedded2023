function [UsedPile, ax] = plotUsedPile3D(edgePile)
    load flatNodeArray.mat flatNodeArray
    load flatNodeArray.mat nodeArray
    load dataGenArgs.mat args
    load newEdgeIdx.mat newEdgeIdx
    load newEdgeTmpCnt.mat newEdgeTmpCnt
    UsedPile = figure("Name", "UsedPile3D");
    set(gcf, 'WindowState', 'maximized');
    ax = gca;
    p = args.P;             % 通道数量
    maxMultiEdgeID = size(newEdgeIdx, 2);
    
    hold on;
    % 调整颜色方案
    func = @(x) colorspace('RGB->RGB',x);
    colors = distinguishable_colors(args.T + 1,'w', func);
    
    for i = 1 : size(edgePile, 1)
        edgeID = i;
        multiEdgeID = 1;    % 重边编号
        singleEdgeID = 1;   % 不计重边时边的编号
        for j = 1 : maxMultiEdgeID
            if ~isempty(find(newEdgeIdx(:, j) == edgeID - 1, 1))
                multiEdgeID = j;
                singleEdgeID = find(newEdgeIdx(:, j) == edgeID - 1, 1);
            end
        end
        startNodeID = edgePile(edgeID, 1) + 1;
        endNodeID = edgePile(edgeID, 2) + 1;
        startX = flatNodeArray{1, startNodeID}.x;
        startY = flatNodeArray{1, startNodeID}.y;
        endX = flatNodeArray{1, endNodeID}.x;
        endY = flatNodeArray{1, endNodeID}.y;
        x = [startX, startX, endX, endX];   % 定义X轴的范围
        y = [startY, startY, endY, endY];   % 定义Y轴的范围
    
        for j = 1 : p
            level = multiEdgeID * (1 / newEdgeTmpCnt(singleEdgeID));
            z = [j - 1 + level - 1 / newEdgeTmpCnt(singleEdgeID), j - 1 + level, j - 1 + level, j - 1 + level - 1 / newEdgeTmpCnt(singleEdgeID)];
            % 使用fill函数填充颜色
            if edgePile(edgeID, 3 + j) == -1
                fill3(x, y, z, colors(1, :), 'FaceAlpha', 1);
            else
                fill3(x, y, z, colors(edgePile(edgeID, 3 + j) + 2, :), 'FaceAlpha', 1);
                % 添加标记
                text((x(1) + x(4)) / 2, (y(1) + y(4)) / 2, (z(1) + z(2)) / 2, num2str(edgePile(edgeID, 3 + j)),'HorizontalAlignment', 'center', 'Color', 'w');
            end

        end
    end

    % 显示图表
    hold off;
    axis off
    % 启用数据提示模式
    dcm = datacursormode(gcf);
    set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
    set(gcf, 'Color', 'white');

end