function [dataGenArgs] = dataGen()
    %% 生成参数配置变量args    
    args = dataGenConfigure();
    %% 生成随机节点
    [nodeArray, flatNodeArray] = nodeCreated(args);
    save flatNodeArray.mat
    
    if args.isVisualization
        SelectedNodes = figure("Name", "Selected Nodes");
        hold on
        for i = 1 : length(flatNodeArray)
            tmpNode = flatNodeArray{1, i};
            if tmpNode.nodeCreated
                plot(tmpNode.x, tmpNode.y, 'b.')
            end
        end
        title("Number of node = " + num2str(args.N));
        axis off;
        set(gcf, 'Color', 'white');
        % 启用数据提示模式
        dcm = datacursormode(gcf);
        set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
        hold off
    end
    %% 求得相应的最小生成树
    startNode = 0;
    edge = Prim(args, flatNodeArray, startNode);    % edge数组的第i行存储第i条边的起点、终点、距离

    if args.isVisualization
        nEdge = size(edge, 1);
        MST = figure("Name", "Minimum spanning tree");
        hold on
        for i = 1 : nEdge
            plot([flatNodeArray{edge(i, 1) + 1}.x, flatNodeArray{edge(i, 2) + 1}.x], ...
            [flatNodeArray{edge(i, 1) + 1}.y, flatNodeArray{edge(i, 2) + 1}.y], ...
            '-ro', 'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end
        title("Number of edge = " + num2str(nEdge));
        axis off;
        set(gcf, 'Color', 'white');
        % 启用数据提示模式
        dcm = datacursormode(gcf);
        set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
        hold off
        axis equal
    end
    
    %% 在相邻的Cell之间添加边，环化MST
    [newEdge] = cyclizeMST(args, nodeArray, flatNodeArray, edge);
    allEdge = [edge; newEdge];
    args.M = size(allEdge, 1);

    if args.isVisualization
        CyclizedMST = figure("Name", "Cyclized MST");
        hold on
        nEdge = size(edge, 1);
        for i = 1 : nEdge
            plot([flatNodeArray{edge(i, 1) + 1}.x, flatNodeArray{edge(i, 2) + 1}.x], ...
            [flatNodeArray{edge(i, 1) + 1}.y, flatNodeArray{edge(i, 2) + 1}.y], ...
            '-ro', 'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end

        nEdge = size(newEdge, 1);
        for i = 1 : nEdge
            plot([flatNodeArray{newEdge(i, 1) + 1}.x, flatNodeArray{newEdge(i, 2) + 1}.x], ...
            [flatNodeArray{newEdge(i, 1) + 1}.y, flatNodeArray{newEdge(i, 2) + 1}.y], ...
            '-go', 'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end
        title("Number of edge = " + num2str(args.M));
        
        h(1) = plot(nan, nan, '-r', 'LineWidth', 1);
        h(2) = plot(nan, nan, '-g', 'LineWidth', 1);
        legend(h, {'MST Edge', 'Cyclized Edge'}, 'Box', 'off', 'FontSize', 10, 'Location', 'northeastoutside');
        axis off;
        set(gcf, 'Color', 'white');
        % 启用数据提示模式
        dcm = datacursormode(gcf);
        set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
        hold off
        axis equal
    end
    %% 随机生成业务
    trans = generateTransaction(args, flatNodeArray, nodeArray);
    %% 输出生成的随机数据
    randDataOutput(args, allEdge, trans);
    %% 读入业务路径（不考虑通道堵塞，此处仅展示业务的分布）
    system('getMinPath.exe');
    % 打开文件
    fileID = fopen('transactionAllocation.txt', 'r');
    
    % 读取第一行
    T = fscanf(fileID, '%d', 1);

    % 预分配一个单元格数组来存储每个业务的数据
    usedEdgeCnt = allEdge;
    usedEdgeCnt(:, 3) = 0;
    
    % 读取每个业务的数据
    for j = 1:T
        % 读取第一个整数pj
        pj = fscanf(fileID, '%d', 1);
        % 读取接下来的pj个整数
        edges = fscanf(fileID, '%d', pj);
        for i = 1 : pj
            usedEdgeCnt(edges(i) + 1, 3) = usedEdgeCnt(edges(i) + 1, 3) + 1;
        end
    end
    % 关闭文件
    fclose(fileID);
    
    if args.isVisualization
        maxUsedEdgeCnt = max(usedEdgeCnt(:, 3));
        cmap = turbo(maxUsedEdgeCnt + 1); % 定义颜色映射（可以根据实际需求选择其他颜色映射）
        TransactionAllocation = figure("Name", "Transaction allocation situation");
        hold on
        % 绘制每条边
        for i = 1 : args.M
            % 根据权值确定颜色深浅
            colorIndex = usedEdgeCnt(i, 3) + 1;
            color = cmap(colorIndex, :);
            lineWidth = 1 + 3 * ((colorIndex + 1) / (maxUsedEdgeCnt + 1));
            plot([flatNodeArray{usedEdgeCnt(i, 1) + 1}.x, flatNodeArray{usedEdgeCnt(i, 2) + 1}.x], ...
            [flatNodeArray{usedEdgeCnt(i, 1) + 1}.y, flatNodeArray{usedEdgeCnt(i, 2) + 1}.y], ...
            '-o', 'Color', color, 'LineWidth', lineWidth, ...
             'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end
        colormap(cmap);
        colorbarHandle = colorbar;
        colorbarHandle.Label.String = 'Number of transactions';
        colorbarHandle.FontSize = 11;
        colorbarHandle.Label.FontSize = 14;
        clim([0, maxUsedEdgeCnt]);
        axis off
        % 启用数据提示模式
        dcm = datacursormode(gcf);
        set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
        set(gcf, 'Color', 'white');
    end
    
    if args.isSaveFigure
        exportgraphics(SelectedNodes, 'SelectedNodes.png', 'Resolution', 600);
        exportgraphics(MST, 'MST.png', 'Resolution', 600);
        exportgraphics(CyclizedMST, 'CyclizedMST.png', 'Resolution', 600);
        exportgraphics(TransactionAllocation, 'TransactionAllocation.png', 'Resolution', 600);
    end

    %% 返回随机数据的参数
    dataGenArgs = args;
    save dataGenArgs.mat
end